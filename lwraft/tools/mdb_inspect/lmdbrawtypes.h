#pragma once

#define FREE_DBI        0
#define MAIN_DBI        1
#define CORE_DBS        2

typedef uint16_t indx_t;
typedef size_t  mdb_size_t;
typedef size_t pgno_t;
typedef size_t txnid_t;
typedef size_t mdb_size_t;
typedef mdb_size_t MDB_ID;


#define NUM_METAS 2

#define MDB_MAGIC        0xBEEFC0DE
#define MDB_DATA_VERSION 1
#define DEFAULT_PAGE_SIZE 4096


typedef struct MDB_db {
        uint32_t        md_pad;         /**< also ksize for LEAF2 pages */
        uint16_t        md_flags;       /**< @ref mdb_dbi_open */
        uint16_t        md_depth;       /**< depth of this tree */
        pgno_t          md_branch_pages;        /**< number of internal pages */
        pgno_t          md_leaf_pages;          /**< number of leaf pages */
        pgno_t          md_overflow_pages;      /**< number of overflow pages */
        mdb_size_t      md_entries;             /**< number of data items */
        pgno_t          md_root;                /**< the root page of this tree
*/
} MDB_db;

typedef struct MDB_page {
#define mp_pgno mp_p.p_pgno
#define mp_next mp_p.p_next
        union {
                pgno_t          p_pgno; /**< page number */
                struct MDB_page *p_next; /**< for in-memory list of freed pages
*/
        } mp_p;
        uint16_t        mp_pad;                 /**< key size if this is a LEAF2
 page */
/**     @defgroup mdb_page      Page Flags
 *      @ingroup internal
 *      Flags for the page headers.
 *      @{
 */
#define P_BRANCH         0x01           /**< branch page */
#define P_LEAF           0x02           /**< leaf page */
#define P_OVERFLOW       0x04           /**< overflow page */
#define P_META           0x08           /**< meta page */
#define P_DIRTY          0x10           /**< dirty page, also set for #P_SUBP pa
ges */
#define P_LEAF2          0x20           /**< for #MDB_DUPFIXED records */
#define P_SUBP           0x40           /**< for #MDB_DUPSORT sub-pages */
#define P_LOOSE          0x4000         /**< page was dirtied then freed, can be
 reused */
#define P_KEEP           0x8000         /**< leave this page alone during spill
*/
/** @} */
        uint16_t        mp_flags;               /**< @ref mdb_page */
#define mp_lower        mp_pb.pb.pb_lower
#define mp_upper        mp_pb.pb.pb_upper
#define mp_pages        mp_pb.pb_pages
        union {
                struct {
                        indx_t          pb_lower;               /**< lower bound
 of free space */
                        indx_t          pb_upper;               /**< upper bound
 of free space */
                } pb;
                uint32_t        pb_pages;       /**< number of overflow pages */
        } mp_pb;
        indx_t          mp_ptrs[1];             /**< dynamic size */
} MDB_page;

/** Size of the page header, excluding dynamic data at the end */
#define PAGEHDRSZ        ((unsigned) offsetof(MDB_page, mp_ptrs))

/** Address of first usable data byte in a page, after the header */
#define METADATA(p)      ((void *)((char *)(p) + PAGEHDRSZ))


typedef struct MDB_meta {
        uint32_t        mm_magic;
        uint32_t        mm_version;
        union {
                MDB_ID  mmun_ull;
                void *mmun_address;
        } mm_un;
#define mm_address mm_un.mmun_address
        mdb_size_t      mm_mapsize;
        MDB_db          mm_dbs[CORE_DBS];
#define mm_psize        mm_dbs[FREE_DBI].md_pad
#define mm_flags        mm_dbs[FREE_DBI].md_flags
        pgno_t          mm_last_pg;
        volatile txnid_t        mm_txnid;
} MDB_meta;


typedef union MDB_metabuf {
        MDB_page        mb_page;
        struct {
                char            mm_pad[PAGEHDRSZ];
                MDB_meta        mm_meta;
        } mb_metabuf;
} MDB_metabuf;

typedef struct MDB_node
{
        unsigned short  mn_lo, mn_hi;

#define F_BIGDATA        0x01
#define F_SUBDATA        0x02
#define F_DUPDATA        0x04

        unsigned short  mn_flags;
        unsigned short  mn_ksize;
        char            mn_data[1];
}MDB_node;
