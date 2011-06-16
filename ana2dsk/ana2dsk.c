
/*
	   readana.c:  convert anadisk format dump files to libdsk format
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "anadisk.h"
#include "libdsk.h"


unsigned char max_sides = 0;
unsigned char max_tracks = 0;
unsigned char max_sectors = 0;
unsigned int sector_size = 0;


/* takes a file handle, file position should be at the start of the Anadisk
   dump data.  reads the info for each sector and builds it into a list
   of sides.  each side holds a list of tracks, and each track a list of sectors */
struct disk_side *parse_image(FILE *fp)
{
	struct sect_info si;
	struct disk_side *sides = NULL;
	struct disk_side *curr_side, *new_side;
	struct disk_track *curr_track, *new_track;
	struct disk_sector *curr_sector, *new_sector;

	
	while (get_sector_info(fp, &si))
	{
		curr_side = find_side(sides, si.phy_side);
		if (curr_side == NULL)
		{
			new_side = make_side(si.phy_side);
			if (sides == NULL)
				sides = new_side;
			else
			{
				for (curr_side = sides; curr_side->next != NULL; curr_side = curr_side->next)
					;
				curr_side->next = new_side;
			}
			max_sides++;
			
			curr_side = new_side;
		}
		
		
		curr_track = find_track(curr_side->tracks, si.phy_cylinder);
		if (curr_track == NULL)
		{
			new_track = make_track(si.phy_cylinder, si.phy_side);
			if (curr_side->tracks == NULL)
				curr_side->tracks = new_track;
			else
			{
				for (curr_track = curr_side->tracks; curr_track->next != NULL; curr_track = curr_track->next)
					;
				curr_track->next = new_track;
			}
			curr_side->track_count++;
			if (curr_side->track_count > max_tracks)
				max_tracks = curr_side->track_count;
			
			curr_track = new_track;
		}
		
		
		new_sector = make_sector(si.log_sector, si.log_cylinder, si.log_side, si.size, ftell(fp));
		if (si.size > sector_size)
			sector_size = si.size;
		if (curr_track->sectors == NULL)
			curr_track->sectors = new_sector;
		else
		{
			for (curr_sector = curr_track->sectors; curr_sector->next != NULL; curr_sector = curr_sector->next)
				;
			curr_sector->next = new_sector;
		}
		curr_track->sector_count++;
		if (curr_track->sector_count > max_sectors)
			max_sectors = curr_track->sector_count;
				
		skip_sector(fp, &si);
	}
	
	return sides;
}



int main(int argc, char *argv[])
{
	FILE *fp;
	struct disk_side *sides, *curr_side;
	struct disk_track *curr_track;
	struct disk_sector *curr_sector;
		
	DSK_GEOMETRY geom;
	DSK_PDRIVER newdisk;
	DSK_FORMAT *format;
	dsk_err_t err;
	
	int i;
	char *secdata;
		
	
	
	if (argc != 3)
	{
		fprintf(stderr, "usage: %s infile outfile\n", argv[0]);
		exit(1);
	}
	
	fp = fopen(argv[1], "r");
	if (fp == NULL)
	{
		fprintf(stderr, "unable to open file: %s\n", argv[1]);
		exit(1);
	}
	
	sides = parse_image(fp);

	memset(&geom, 0, sizeof(DSK_GEOMETRY));
	geom.dg_cylinders = max_tracks;
	geom.dg_heads = max_sides;
	geom.dg_sectors = max_sectors;
	geom.dg_secbase = 1;  /* perhaps this should be determined from file? */
	geom.dg_secsize = sector_size;
	
	format = malloc(geom.dg_sectors*sizeof(DSK_FORMAT));
	secdata = malloc(geom.dg_secsize);
	if ((format == NULL) || (secdata == NULL))
	{
		fprintf(stderr, "main: out of memory\n");
		exit(1);
	}
	
	
	if (dsk_creat(&newdisk, argv[2], "dsk", NULL) != DSK_ERR_OK)
	{
		fprintf(stderr, "main: error in dsk_creat\n");
		exit(1);
	}

    for (i=0; i<max_sides; i++)
		if (dsk_apform(newdisk, &geom, 0, i, 0xE5) != DSK_ERR_OK)
		{
			fprintf(stderr, "main: error formatting inital cylinder\n");
			exit(1);
		}
	
	curr_side = sides;
	while (curr_side != NULL)
	{
/*	printf("Side = %i\n", curr_side->id); */
		curr_track = curr_side->tracks;
		
		while (curr_track != NULL)
		{
			/*printf("   Track = %i\n", curr_track->id); */
			curr_sector = curr_track->sectors;
			i = 0;
			while (curr_sector != NULL)
			{
			/*	printf("      Sector = %i\n", curr_sector->id);*/
				format[i].fmt_cylinder = curr_sector->logical_track;
				format[i].fmt_head = curr_sector->logical_side;
				format[i].fmt_sector = curr_sector->id;
				format[i].fmt_secsize = curr_sector->length;
				
				curr_sector = curr_sector->next;
				i++;
			}
			
			if ((err = dsk_pformat(newdisk, &geom, curr_track->id, curr_side->id, format, 0xE5)) != DSK_ERR_OK)
			{
				fprintf(stderr, "main: error in dsk_pformat(%i)\n", err);
				exit(1);
			}
			
			curr_sector = curr_track->sectors;
			while (curr_sector != NULL)
			{
				fseek(fp, curr_sector->ofs, SEEK_SET);
				if (fread(secdata, 1, curr_sector->length, fp) != curr_sector->length)
				{
					fprintf(stderr, "main: error reading input file sector\n");
					exit(1);
				}
				if ((err = dsk_xwrite(newdisk, &geom, secdata, curr_track->id, curr_side->id, curr_sector->logical_track, curr_sector->logical_side, curr_sector->id, curr_sector->length, 0)) != DSK_ERR_OK)
				{
					fprintf(stderr, "main: error in dsk_pwrite (%i, sec=%i, side=%i, track=%i) \n", err, curr_sector->id, curr_sector->logical_side, curr_sector->logical_track);
					exit(1);
				}
				curr_sector = curr_sector->next;
			}
				
			curr_track = curr_track->next;
		}
		
		curr_side = curr_side->next;
	}
	
	

	printf("Sides: %2i  Tracks: %2i  Sectors: %2i\n", max_sides, max_tracks, max_sectors);

	
	dsk_close(&newdisk);
	fclose(fp);
	return 0;
}
