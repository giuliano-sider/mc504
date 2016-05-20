#ifndef K_HASHMAP_____
#define K_HASHMAP_____

/******************************************************************************************
 * implementation of hash map data structure,
 * mapping unique integer keys to strings (char*)
 */


/* collection of primes bigger than powers of 2, 0 to 30 */
static const int prime_capacities[] = {
	2,
	3,
	7,
	13,
	29,
	53,
	97,
	193,
	389,
	769,
	1543,
	3079,
	6151,
	12289,
	24593,
	49157,
	98317,
	196613,
	393241,
	786433,
	1572869,
	3145739,
	6291469,
	12582917,
	25165843,
	50331653,
	100663319,
	201326611,
	402653189,
	805306457,
	1610612741
};

static const int tabletoosmall[] = { /* 1/4 of capacity -> must decrease table size to smaller (prime) number of buckets */
	0		    ,
	0                   ,
	1                   ,
	3                   ,
	7                   ,
	13                  ,
	24                  ,
	48                  ,
	97                  ,
	192                 ,
	385                 ,
	769                 ,
	1537                ,
	3072                ,
	6148                ,
	12289               ,
	24579               ,
	49153               ,
	98310               ,
	196608              ,
	393217              ,
	786434              ,
	1572867             ,
	3145729             ,
	6291460             ,
	12582913            ,
	25165829            ,
	50331652            ,
	100663297           ,
	201326614           ,
	402653185           
};

static const int tabletoobig[] = { /* 3/4 of capacity -> must increase table size to larger (prime) number of buckets */
	2		    ,
	2                   ,
	5                   ,
	9                   ,
	21                  ,
	39                  ,
	72                  ,
	144                 ,
	291                 ,
	576                 ,
	1157                ,
	2309                ,
	4613                ,
	9216                ,
	18444               ,
	36867               ,
	73737               ,
	147459              ,
	294930              ,
	589824              ,
	1179651             ,
	2359304             ,
	4718601             ,
	9437187             ,
	18874382            ,
	37748739            ,
	75497489            ,
	150994958           ,
	301989891           ,
	603979842           ,
	1207959555          
};

/* these have the disadvantage of being close to powers of 2
static int primes[] = {
	2,
	3,
	5,
	11,
	17,
	37,
	67,
	131,
	257,
	521,
	1031,
	2053,
	4099,
	8209,
	16411,
	32771,
	65537,
	131101,
	262147,
	524309,
	1048583,
	2097169,
	4194319,
	8388617,
	16777259,
	33554467,
	67108879,
	134217757,
	268435459,
	536870923,
	1073741827
};
*/

typedef struct linked_list_int_string {
	struct linked_list_int_string *next;
	int key;
	char *value;
} list_t, map_entry_t;

static map_entry_t sentinel_node = {NULL, 42, NULL};

typedef struct int_string_map {
	list_t **table; /* array of buckets (pointers to lists of key-value pairs); we resolve collisions by chaining */
	int mappings; /* number of key value pairs */
	int capacity; /* index into the table that tells us the capacity of the map (number of buckets) */ 
	int (*hashfunc)(struct int_string_map*, int); /* use the default implementation (key modulo a prime) or roll your own */
} map_t;

/* we will strive to keep the table's load factor in the range below
 * we define load factor as map->mappings / prime_capacities[map->capacity]
 */
#define LOAD_FACTOR_MIN 0.25
#define LOAD_FACTOR_MAX 0.75


/* table will never be made smaller than prime_capacities[10] and bigger than prime_capacities[30] */
#define MIN_TABLE_SIZE_INDEX 10
#define MIN_TABLE_SIZE prime_capacities[MIN_TABLE_SIZE_INDEX]
#define MAX_TABLE_SIZE_INDEX 30
#define MAX_TABLE_SIZE prime_capacities[MAX_TABLE_SIZE_INDEX]

static map_t *construct_map(int (*hashfunc)(map_t*, int) );
static void destroy_map(map_t *map);
static void set_hashfunc(map_t *map, int (*hashfunc)(map_t*, int) );
static int default_hashfunc(map_t *map, int key);
static char *get_mapping(map_t *map, int key);
static char *remove_mapping(map_t *map, int key); 
static int put_mapping(map_t *map, int key, char *value);
static void adjust_map_size(map_t *map);

/* opaque iterator data structure to point to a hash map element */
typedef struct map_iterator_t {
	map_t *map_; /* private. map associated with this iterator */
	int bucket_; /* private. current bucket in the iteration */
	map_entry_t *e_; /* private. current mapping in the iteration */
} map_iterator_t;

static map_iterator_t construct_map_iterator(map_t *map, map_iterator_t *it);
static inline int get_entry_key(map_iterator_t it);
static inline char *get_entry_value(map_iterator_t it);
static inline void set_entry_value(map_iterator_t it, char *val);
static inline int has_valid_entry(map_iterator_t it);
static inline void iterate_next(map_iterator_t *it);

static void print_hash_map(map_t *map);

#endif /* K_HASHMAP_____ */
