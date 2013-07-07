#ifndef UPARTSFS_H
#define UPARTSFS_H

#include <glib.h>

#define UPARTS_NAME_LENGTH 42

/*AJN Copied here for quick reference*/
/*
    struct TSK_VS_PART_INFO {
        int tag;
        TSK_VS_PART_INFO *prev; ///< Pointer to previous partition (or NULL)
        TSK_VS_PART_INFO *next; ///< Pointer to next partition (or NULL)
        TSK_VS_INFO *vs;        ///< Pointer to parent volume system handle

        TSK_DADDR_T start;      ///< Sector offset of start of partition
        TSK_DADDR_T len;        ///< Number of sectors in partition
        char *desc;             ///< UTF-8 description of partition (volume system type-specific)
        int8_t table_num;       ///< Table address that describes this partition
        int8_t slot_num;        ///< Entry in the table that describes this partition
        TSK_PNUM_T addr;        ///< Address of this partition
        TSK_VS_PART_FLAG_ENUM flags;    ///< Flags for partition
    };
*/


struct UPARTS_DE_INFO {
	TSK_DADDR_T offset;
	TSK_DADDR_T length;
	struct stat st;
	uint8_t encounter_order;
	char name[UPARTS_NAME_LENGTH];
};

struct UPARTS_EXTRA {
	GList *stats_by_offset;
	GList *stats_by_index;
	TSK_IMG_INFO *tsk_img;
	TSK_VS_INFO *tsk_vs;
};

static struct UPARTS_EXTRA *uparts_extra;

int free_uparts_extra(struct UPARTS_EXTRA *);

#endif //UPARTSFS_H
