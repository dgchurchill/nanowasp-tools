#ifndef ANADISK_H
#define ANADISK_H

#include <stdio.h>

struct sect_info
{
	unsigned char phy_cylinder;
	unsigned char phy_side;
	unsigned char log_cylinder;
	unsigned char log_side;
	unsigned char log_sector;
	unsigned char length_code;
	unsigned int size;
};


struct disk_sector
{
	struct disk_sector *next;
	unsigned char id;
	unsigned char logical_track;
	unsigned char logical_side;
	unsigned int length;
	unsigned long ofs;
};

struct disk_track
{
	struct disk_track *next;
	struct disk_sector *sectors;
	unsigned int sector_count;
	unsigned char id;
	unsigned char side_id;
};

struct disk_side
{
	struct disk_side *next;
	struct disk_track *tracks;
	unsigned int track_count;
	unsigned char id;
};


struct disk_side *find_side(struct disk_side *sides, unsigned char id);
struct disk_track *find_track(struct disk_track *tracks, unsigned char id);
struct disk_side *make_side(unsigned char id);
struct disk_track *make_track(unsigned char id, unsigned char side_id);
struct disk_sector *make_sector(unsigned char id, unsigned char logical_track, unsigned char logical_side, unsigned int length, unsigned long ofs);
int get_sector_info(FILE *fp, struct sect_info *si);
inline void skip_sector(FILE *fp, struct sect_info *si);
unsigned char *get_sector(FILE *fp, struct sect_info *si);



#endif /* ANADISK_H */
