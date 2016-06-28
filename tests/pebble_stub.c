#include <pebble.h>
#include <linked-list/linked-list.h>
#include "constants.h"

static LinkedRoot* s_persist_root = NULL;

typedef struct SimpleTuplet {
  uint32_t key;
  union {
    struct {
      uint8_t *data;
      uint16_t length;
    } bytes;
    uint32_t integer;
  };
} SimpleTuplet;

#define SimpleTupletBytes(_key, _data, _length) \
((const SimpleTuplet) { .key = _key, .bytes = { .data = _data, .length = _length }})

#define SimpleTupletInteger(_key, _integer) \
((const SimpleTuplet) { .key = _key, .integer =  _integer })

static bool prv_tuplet_compare(void* object1, void* object2){
	SimpleTuplet* t1 = (SimpleTuplet*)object1;
	SimpleTuplet* t2 = (SimpleTuplet*)object2;
	return t1->key == t2->key;
}

bool persist_exists(const uint32_t key){
	if(!s_persist_root){
		s_persist_root = linked_list_create_root();
	}
	SimpleTuplet t = { .key = key };
	return linked_list_contains_compare(s_persist_root, &t, prv_tuplet_compare);
}

int32_t persist_read_int(const uint32_t key){
	if(!s_persist_root){
		s_persist_root = linked_list_create_root();
	}
	SimpleTuplet t = { .key = key };
	int16_t index = linked_list_find_compare(s_persist_root, &t, prv_tuplet_compare);
	if(index != -1){
		SimpleTuplet* res = (SimpleTuplet*)linked_list_get(s_persist_root, index);
		return res->integer;
	}
	return 0;
}

int persist_read_data(const uint32_t key, void *buffer, const size_t buffer_size){
	if(!s_persist_root){
		s_persist_root = linked_list_create_root();
	}
	SimpleTuplet t = { .key = key };
	int16_t index = linked_list_find_compare(s_persist_root, &t, prv_tuplet_compare);
	if(index != -1){
		SimpleTuplet* res = (SimpleTuplet*)linked_list_get(s_persist_root, index);
		memcpy(buffer, res->bytes.data, buffer_size);
		return res->bytes.length;
	}
	return 0;
}

status_t persist_write_int(const uint32_t key, const int32_t value){
	if(!s_persist_root){
		s_persist_root = linked_list_create_root();
	}
	SimpleTuplet t = SimpleTupletInteger(key, value);
	int16_t index = linked_list_find_compare(s_persist_root, &t, prv_tuplet_compare);
	if(index != -1){
		SimpleTuplet* res = (SimpleTuplet*)linked_list_get(s_persist_root, index);
		res->integer = value;
	}
	else {
		SimpleTuplet* res = malloc(sizeof(SimpleTuplet));
		memcpy(res, &t, sizeof(SimpleTuplet));
		linked_list_append(s_persist_root, res);
	}
	return 0;
}

int persist_write_data(const uint32_t key, const void *data, const size_t size){
	if(!s_persist_root){
		s_persist_root = linked_list_create_root();
	}
	SimpleTuplet t = SimpleTupletBytes(key, 0, 0);
	int16_t index = linked_list_find_compare(s_persist_root, &t, prv_tuplet_compare);
	if(index != -1){
		SimpleTuplet* res = (SimpleTuplet*)linked_list_get(s_persist_root, index);
		if(res->bytes.data)
			free(res->bytes.data);
		res->bytes.data = malloc(size);
		res->bytes.length = size;
		memcpy(res->bytes.data, data, size);
	}
	else {
		SimpleTuplet* res = malloc(sizeof(SimpleTuplet));
		memcpy(res, &t, sizeof(SimpleTuplet));
		res->bytes.data = malloc(size);
		res->bytes.length = size;
		memcpy(res->bytes.data, data, size);
		linked_list_append(s_persist_root, res);
	}
	return size;
}

// struct Dictionary;
typedef struct Dictionary {
	LinkedRoot* root;
	uint16_t index;
	const uint8_t *buffer;
	uint16_t size;
	uint16_t offset;
} Dictionary;

static bool prv_tuple_compare(void* object1, void* object2){
	Tuple* t1 = (Tuple*)object1;
	Tuple* t2 = (Tuple*)object2;
	return t1->key == t2->key;
}

static void prv_init_dict(DictionaryIterator *iter){
	if(!iter->dictionary){
		iter->dictionary = malloc(sizeof(Dictionary));
		iter->dictionary->root = linked_list_create_root();
		iter->dictionary->index = 0;
	}
}

uint32_t dict_size(DictionaryIterator* iter){
	if(iter){
		prv_init_dict(iter);
	}
	return TUPLE_SIZE * linked_list_count(iter->dictionary->root);
}

DictionaryResult dict_write_begin(DictionaryIterator *iter, uint8_t * const buffer, const uint16_t size){
	if(iter){
		prv_init_dict(iter);
	}

	linked_list_clear(iter->dictionary->root);
	iter->dictionary->index = 0;
	iter->dictionary->buffer = buffer;
	iter->dictionary->size = size;
	iter->dictionary->offset = 0;

	return 0;
}

uint32_t dict_write_end(DictionaryIterator *iter){
	if(iter){
		prv_init_dict(iter);
		return TUPLE_SIZE * linked_list_count(iter->dictionary->root);
	}
	return 0;
}

Tuple * dict_read_begin_from_buffer(DictionaryIterator *iter, const uint8_t * const buffer, const uint16_t size){
	if(iter){
		prv_init_dict(iter);

		linked_list_clear(iter->dictionary->root);
		iter->dictionary->index = 0;
		iter->dictionary->buffer = buffer;
		iter->dictionary->size = size;
		iter->dictionary->offset = 0;
	
		uint16_t count = size / TUPLE_SIZE;
		for(uint16_t i=0; i<count; i++){
			Tuple* t=(Tuple*)(buffer + i*TUPLE_SIZE);
			linked_list_append(iter->dictionary->root, t);
		}
	}
	return NULL;
}

Tuple * dict_read_next(DictionaryIterator *iter){
	if(iter){
		prv_init_dict(iter);
		uint16_t count = linked_list_count(iter->dictionary->root);
		if(count > 0 && iter->dictionary->index < count){
			return linked_list_get(iter->dictionary->root, iter->dictionary->index++);
		}
	}
	return NULL;
}

Tuple * dict_read_first(DictionaryIterator *iter){
	if(iter){
		prv_init_dict(iter);
		uint16_t count = linked_list_count(iter->dictionary->root);
		if(count > 0){
			iter->dictionary->index = 0;
			return linked_list_get(iter->dictionary->root, iter->dictionary->index++);
		}
	}
	return NULL;
}	

DictionaryResult dict_merge(DictionaryIterator *dest, uint32_t *dest_max_size_in_out,
                             DictionaryIterator *source,
                             const bool update_existing_keys_only,
                             const DictionaryKeyUpdatedCallback key_callback, void *context){
	uint16_t count = linked_list_count(source->dictionary->root);
	for(uint16_t i=0; i<count; i++){
		Tuple* t=(Tuple*)linked_list_get(source->dictionary->root, i);
		int16_t destindex = linked_list_find_compare(dest->dictionary->root, t, prv_tuple_compare);
		Tuple* t2=NULL;
		if(destindex != -1){
			t2=(Tuple*)linked_list_get(dest->dictionary->root, destindex);
			memcpy(t2, t, TUPLE_SIZE);
		}
		else {
			if(dest->dictionary->offset < dest->dictionary->size){
				t2=(Tuple*)(dest->dictionary->buffer + dest->dictionary->offset);
				dest->dictionary->offset += TUPLE_SIZE;
				memcpy(t2, t, TUPLE_SIZE);
				linked_list_append(dest->dictionary->root, t2);
			}
		}
	}
	return 0;
}

Tuple *dict_find(const DictionaryIterator *iter, const uint32_t key){
	if(iter && iter->dictionary && iter->dictionary->root){
		Tuple t;
		t.key = key;
		int16_t index = linked_list_find_compare(iter->dictionary->root, &t, prv_tuple_compare);
		if(index != -1){
			return (Tuple*)linked_list_get(iter->dictionary->root, index);
		}
	}
	return 0;
}

DictionaryResult dict_write_int32(DictionaryIterator * iter, const uint32_t key, const int32_t value){
	if(iter){
		prv_init_dict(iter);
		Tuple t;
		t.key = key;
		int16_t index = linked_list_find_compare(iter->dictionary->root, &t, prv_tuple_compare);
		if(index != -1){
			Tuple* res = (Tuple*)linked_list_get(iter->dictionary->root, index);
			res->value->uint32 = value;
		}
		else if(iter->dictionary->offset < iter->dictionary->size){
			Tuple* res=(Tuple*)(iter->dictionary->buffer + iter->dictionary->offset);
			iter->dictionary->offset += TUPLE_SIZE;
			res->key = key;
			res->value->int32 = value;
			linked_list_append(iter->dictionary->root, res);
		}
	}
	return 0;
}

DictionaryResult dict_write_cstring(DictionaryIterator * iter, const uint32_t key, const char *const cstring){
	if(iter){
		prv_init_dict(iter);
		Tuple t;
		t.key = key;
		int16_t index = linked_list_find_compare(iter->dictionary->root, &t, prv_tuple_compare);
		if(index != -1){
			Tuple* res = (Tuple*)linked_list_get(iter->dictionary->root, index);
			linked_list_remove(iter->dictionary->root, index);
			free(res);
		}
		else if(iter->dictionary->offset < iter->dictionary->size){
			Tuple* res=(Tuple*)(iter->dictionary->buffer + iter->dictionary->offset);
			iter->dictionary->offset += TUPLE_SIZE;
			res->key = key;
			strcpy(res->value->cstring, cstring);
			linked_list_append(iter->dictionary->root, res);
		}		
	}
	return 0;
}
