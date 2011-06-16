#include "anadisk.h"
#include <stdio.h>
#include <stdlib.h>

struct disk_side *find_side(struct disk_side *sides, unsigned char id)
{
	for (; sides != NULL; sides = sides->next)
		if (sides->id == id)
			return sides;
		
	return NULL;
}


struct disk_track *find_track(struct disk_track *tracks, unsigned char id)
{
	for (; tracks != NULL; tracks = tracks->next)
		if (tracks->id == id)
			return tracks;
		
	return NULL;
}


struct disk_side *make_side(unsigned char id)
{
	struct disk_side *new;
		
	new = malloc(sizeof(struct disk_side));
	if (new == NULL)
	{
		fprintf(stderr, "new_side: out of memory\n");
		exit(1);
	}
	
	new->id = id;
	new->track_count = 0;
	new->tracks = NULL;
	new->next = NULL;
	
	return new;
}


struct disk_track *make_track(unsigned char id, unsigned char side_id)
{
	struct disk_track *new;
		
	new = malloc(sizeof(struct disk_track));
	if (new == NULL)
	{
		fprintf(stderr, "new_track: out of memory\n");
		exit(1);
	}
	
	new->id = id;
	new->side_id = side_id;
	new->sector_count = 0;
	new->sectors = NULL;
	new->next = NULL;
	
	return new;
}


struct disk_sector *make_sector(unsigned char id, unsigned char logical_track, unsigned char logical_side, unsigned int length, unsigned long ofs)
{
	struct disk_sector *new;
		
	new = malloc(sizeof(struct disk_sector));
	if (new == NULL)
	{
		fprintf(stderr, "new_sector: out of memory\n");
		exit(1);
	}
	
	new->id = id;
	new->logical_track = logical_track;
	new->logical_side = logical_side;
	new->length = length;
	new->ofs = ofs;
	new->next = NULL;
	
	return new;
}



int get_sector_info(FILE *fp, struct sect_info *si)
{
	unsigned char inf[8];

	if (fread(inf, 1, 8, fp) != 8)
	{
		if (feof(fp))
			return 0;
		
		fprintf(stderr, "get_sector: bad sector header\n");
		exit(1);
	}
	
	si->phy_cylinder = inf[0];
	si->phy_side = inf[1];
	si->log_cylinder = inf[2];
	si->log_side = inf[3];
	si->log_sector = inf[4];
	si->length_code = inf[5];
	si->size = inf[6] + 256*(int)inf[7];
	
	return 1;
}


inline void skip_sector(FILE *fp, struct sect_info *si)
{
	fseek(fp, si->size, SEEK_CUR);
}


unsigned char *get_sector(FILE *fp, struct sect_info *si)
{
	unsigned char *data;
	
	if (!get_sector_info(fp, si))   /* end of file */
		return NULL;
	
	data = malloc(si->size);
	if (data == NULL)
	{
		fprintf(stderr, "get_sector: out of memory\n");
		exit(1);
	}
	
	if (fread(data, 1, si->size, fp) != si->size)
	{
		free(data);
		fprintf(stderr, "get_sector: missing sector data\n");
		exit(1);
	}
	
	return data;
}
