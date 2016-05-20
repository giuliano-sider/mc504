#include <linux/unistd.h>
#include <linux/linkage.h>
#include <linux/slab.h>

#include <asm/uaccess.h>

#include "getsetkey.h"

/******************************************************************************************
 * implementation of hash map data structure,
 * mapping unique integer keys to strings (char*)
 * the map has adjustable size, implements search, insertion and deletion.
 * iterator structure supports iteration (for printing the map, as an example)
 */

/* construct an iterator pointing to same mapping as *it, or to the
 * first element found in the hash map, *map, in case NULL rather than *it.
 * either argument can be NULL depending on which constructor is desired (but not both)
 * if elements are inserted or deleted from the map during an 
 * iteration, the validity of the iterator is undefined.
 */
static map_iterator_t construct_map_iterator(map_t *map, map_iterator_t *it) {
	
	map_iterator_t i;
	if (it == NULL) {  /* iterator points to first mapping in the table */
		i.map_ = map;
		if (map->mappings == 0) {
			i.bucket_ = -1;
			i.e_ = &sentinel_node;
		} else { /* find the first mapping */
			int j;
			for (j = 0; j < prime_capacities[map->capacity]; j++) {
				if (map->table[j] != &sentinel_node) { 
					i.bucket_ = j;
					i.e_ = map->table[j];
					break;
				}
			}
		}
	} else { /* copy constructor */
		i.map_ = it->map_;
		i.bucket_ = it->bucket_;
		i.e_ = it->e_;
	}

	return i;
}

static inline int get_entry_key(map_iterator_t it) {
	return it.e_->key;
}
static inline char *get_entry_value(map_iterator_t it) {
	return it.e_->value;
}
static inline void set_entry_value(map_iterator_t it, char *val) {
	it.e_->value = val;
}
/* returns 1 if it points to an element, zero if it points to the sentinel node */
static inline int has_valid_entry(map_iterator_t it) {
	return it.e_ == &sentinel_node ? 0 : 1;
}
/* makes iterator point to the next element in the table (or the sentinel at the end)
 * results are undefined if the given iterator doesn't point to any element */
static void iterate_next(map_iterator_t *it) {
	it->e_ = it->e_->next;
	if (it->e_ == &sentinel_node) { /* must check next bucket in the table */
		for (it->bucket_++; it->bucket_ < prime_capacities[it->map_->capacity]; it->bucket_++) {
			if (it->map_->table[it->bucket_] != &sentinel_node) {
				it->e_ = it->map_->table[it->bucket_];
				break;
			}
		}
	}
}

/* constructs an empty map whose hash function is given as a parameter,
 * (if NULL is specified, the default hash function is used)
 * and capacity of primes[10] (between 2^10 and 2^11) 
 * returns NULL if allocation is unsuccessful
 */
static map_t *construct_map(int (*hashfunc)(map_t*, int) ) {
	
	int i;
	map_t *map;
	
	if ((map = kmalloc(sizeof(map_t), GFP_KERNEL )) == NULL)
		return NULL;
	if ((map->table = kmalloc(sizeof(list_t *) * MIN_TABLE_SIZE, GFP_KERNEL) ) == NULL ) {
		kfree(map);
		return NULL;
	}
	
	map->mappings = 0;
	map->capacity = MIN_TABLE_SIZE_INDEX;
	
	for (i = 0; i < prime_capacities[map->capacity]; i++)
		map->table[i] = &sentinel_node; /* we use a sentinel in place of the classic NULL */
	
	set_hashfunc(map, hashfunc);

	return map;
}

/* destroys the map. client is responsible for avoiding dangling pointers
 * and for deallocating contents of the table
 */
static void destroy_map(map_t *map) {
	int i;
	for (i = 0; i < prime_capacities[map->capacity]; i++) {
		list_t *bucket = map->table[i];
		while (bucket != &sentinel_node) {
			list_t *temp = bucket->next;
			kfree(bucket);
			bucket = temp;
		}
	}
	kfree(map->table);
	kfree(map);
}

/* specifies the hash function to be used to turn integer keys
 * into hash table indices
 */
static void set_hashfunc(map_t *map, int (*hashfunc)(map_t*, int) ) {
	if (hashfunc == NULL)
		hashfunc = default_hashfunc;
	map->hashfunc = hashfunc;
}

/* returns the index of a key in the hash table 
 * we use a trivial 'modulo a prime' hash function 
 * (far from powers of 2 in order to mix up the bits)
 */
static int default_hashfunc(map_t *map, int key) {
	return ((unsigned int)key) % prime_capacities[map->capacity]; 
}


/* return the string associated with this particular integer key, 
 * or NULL if there is none 
 */
static char *get_mapping(map_t *map, int key) {
	
	list_t *bucket;

	if (map == NULL)
		return NULL;

	sentinel_node.key = key;
	bucket = map->table[map->hashfunc(map, key)];
	while (bucket->key != key)
		bucket = bucket->next;
	
	return bucket->value; /* note: the sentinel's value attribute is always NULL */
}

/* removes the mapping associated with key,
 * returning the old value associated with key,
 * or NULL if there was none
 */
static char *remove_mapping(map_t *map, int key) {
	
	list_t **previous_ptr;

	if (map == NULL)
		return NULL;

	sentinel_node.key = key; /* store an elephant in cairo */

	/* thou shalt use double pointers lest thou faceth Lord Linus's wrath */
	previous_ptr = &map->table[map->hashfunc(map, key)]; /* store address of ptr to the appropriate list */
	while ((*previous_ptr)->key != key) /* while key not found */
		previous_ptr = &((*previous_ptr)->next); /* update address of next ptr in the node we just explored */
	
	if (*previous_ptr == &sentinel_node)
		return NULL; /* key not found */
	else { 
		char *returnval = (*previous_ptr)->value; /* 1. save value paired with found key */
		list_t *temp = *previous_ptr; /* 2. save ptr to node we will delete */
		*previous_ptr = (*previous_ptr)->next; /* 3. set next pointer of preceding node to skip deleted node */
		kfree(temp); /* 4. delete node */

		/* check to see if we should shrink the table */
		map->mappings--;
		adjust_map_size(map);

		return returnval; /* key found */
	}

}

/* attempts to insert the mapping (key, value) in map,
 * returning -1 in the event of failure,
 * 0 if the insertion was successful and key was not previously mapped to anything
 * or 1 if the key already belonged and its mapping was reset.
 * if NULL is passed as the value, the mapping associated with key is removed.
 */
static int put_mapping(map_t *map, int key, char *value) {
	
	list_t **previous_ptr;

	if (map == NULL)
		return -1;

	sentinel_node.key = key;
	/* double pointer trickery allows us to modify the next pointer of a previous node */
	/* not using double pointers puts us at risk of being flamed by the terrible thorvalds */
	previous_ptr = &map->table[map->hashfunc(map, key)];
	/* ptr to next ptr initialized to addr of beginning of key's bucket */
	while ((*previous_ptr)->key != key)
		previous_ptr = &((*previous_ptr)->next);
	/* while we don't bump into the key, set ptr to ptr to point to the node's next ptr*/
	if (*previous_ptr != &sentinel_node) {
		if (value != NULL)
			(*previous_ptr)->value = value; /* key already belonged: mapping is reset */
		else /* remove the mapping when user value is NULL */
			remove_mapping(map, key);
		adjust_map_size(map);
		return 1; /* reset mapping */
	} else { /* insert key: it's guaranteed to be unique in this map */
		list_t *newnode;
		if (value == NULL) /* NULL value is used to remove the mapping */
			return -1;
		newnode = kmalloc(sizeof(list_t), GFP_KERNEL);
		if (newnode == NULL)
			return -1; /* failed allocation */
		newnode->key = key;
		newnode->value = value;
		newnode->next = &sentinel_node;
		*previous_ptr = newnode;

		/* check if the size of the table should increase */
		map->mappings++;
		adjust_map_size(map);

		return 0;
	}

}
/* used internally by the put_mapping and remove_mapping routines to ensure that the
 * map has a load factor between 0.25 (rather sparse) and 0.75 (getting quite packed)
 */
static void adjust_map_size(map_t *map) {
	
	int i;
	list_t **new_table;
	int old_capacity = prime_capacities[map->capacity];

	/* check if we should shrink or increase the table */
	if (map->capacity > MIN_TABLE_SIZE_INDEX && map->mappings < tabletoosmall[map->capacity]) {
		map->capacity--;
	} else if (map->capacity < MAX_TABLE_SIZE_INDEX && map->mappings > tabletoobig[map->capacity]) {
		map->capacity++;
	} else
		return;

	new_table = kmalloc(sizeof(list_t *) * prime_capacities[map->capacity], GFP_KERNEL);
	if (new_table == NULL )
		return; /* allocation failed. just abort and return */

	for (i = 0; i < prime_capacities[map->capacity]; i++)
		new_table[i] = &sentinel_node; /* sentinel in place of the classic NULL */

	for(i = 0; i < old_capacity; i++) { /* stop the world: evacuate everybody to the new table */
		list_t *bucket = map->table[i];
		while (bucket != &sentinel_node) {
			int new_index = map->hashfunc(map, bucket->key);
			list_t *temp = bucket->next;
			bucket->next = new_table[new_index];
			new_table[new_index] = bucket;
			bucket = temp;
		}
	}
	kfree(map->table);
	map->table = new_table;

}

/* prints all the mappings in the table as a depth 1 JSON string */
static void print_hash_map(map_t *map) {
	map_iterator_t i = construct_map_iterator(map, NULL);
	printk("{\n");
	while (has_valid_entry(i)) {
		/* note that we don't consider the case of quotes inside the string, or 
		 * bad formatting or anything else like that, making the JSON unsafe to use
		 */
		printk("\t\"%i\": \"%s\"\n", get_entry_key(i), get_entry_value(i));
		iterate_next(&i);
	}
	printk("}\n");
}


/* this is our initially empty (and uninitialized hash table)
 */
static map_t *hash_table = NULL;


/* return the string (by putting it in *value) associated with this particular integer key, 
 * or NULL if there is none 
 */
asmlinkage long sys_getkey(int key, char **value) {
	char *s = get_mapping(hash_table, key);
	int missing;
	if ((missing = copy_to_user(value, &s, sizeof(char*))) != 0) { /* store a pointer to mapped value in the user's variable */
		printk("sys_getkey failed at line 305. get_mapping returned %p.\n\
			apparently %i bytes failed to be transferred to the user's memory\n", s, missing);
		return -1;
	}
	return 0;
}

/* attempts to insert the mapping (key, value) in map,
 * returning -1 in the event of failure,
 * 0 if the insertion was successful and key was not previously mapped to anything
 * or 1 if the key already belonged and its mapping was reset.
 * if NULL is passed as the value, the mapping associated with key is removed.
 */
asmlinkage long sys_setkey(int key, char *value) {
	if (hash_table == NULL)
		hash_table = construct_map(default_hashfunc);
	return put_mapping(hash_table, key, value);
}
