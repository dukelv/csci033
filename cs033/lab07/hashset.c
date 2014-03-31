#include <stdlib.h>
#include <stdio.h>

#include "hashset.h"

#define LOAD_FACTOR 0.75f  // The ratio of elements to buckets at which a rehash should occur
#define INIT_CAP 4  // Initial number of buckets

/* A BucketNode is a node in a singly-linked list */
typedef struct BucketNode BucketNode;
struct BucketNode {
	void *value;
	BucketNode *next;
};

struct HashSet {
	int (*hashcode)(void *);  // Function to get an element's hashcode
	int (*equals)(void *, void *);  // Function to compare two elements for equality
	void *(*copy)(void *);  // Function to copy an element
	void (*delete)(void *);  // Function to delete an element

	BucketNode **buckets;  // The bucket array
	int nbuckets;  // Size of the bucket array
	int size;  // Number of elements in the set
};

/******************************
 * Creation/deletion functions
 ******************************/

/* Create a new hashset with the given element functions and return it */
HashSet *hashset_new(int (*hashcode)(void *), int (*equals)(void *, void *), void *(*copy)(void *), void (*delete)(void *)) 
{
    HashSet *set = (HashSet *) malloc(sizeof(HashSet));
	// TODO: 1. Allocate space for a HashSet struct.
	//       2. Initialize the struct, including the
	//          internal bucket array.
    set->hashcode = hashcode;
    set->equals = equals;
    set->copy = copy;
    set->delete = delete;
    set->nbuckets = INIT_CAP;
    set->buckets = (BucketNode **)malloc(INIT_CAP*sizeof(BucketNode*));
    int i;
    for(i = 0; i < INIT_CAP; i++)
    {
	(set->buckets)[i] = NULL;
	//(set->buckets)[i]->next = NULL;
	//(set->buckets)[i]->value = NULL;
    }
    set->size = 0;
    return set;
}

/* Delete the given hashset */
void hashset_delete(HashSet *hashset) {
	// TODO: 1. Delete all of the hashset's internal data.
	//       2. Free the struct.
	hashset_clear(hashset);
	int i = 0;
	for(i = 0; i < (hashset->nbuckets); i++)
	{
	    free((hashset->buckets)[i]);
	}
	free(hashset->buckets);
	free(hashset);
}


/******************************
 * Access/modification functions
 ******************************/

/* Add a copy of the given element to the set
 * Return 1 if the element was added successfully
 * 0 otherwise (if the element was already present)
 */
int hashset_add(HashSet *hashset, void *element) {
	// TODO: 1. Determine the bucket for the given element.
	int bucket_number = hashset->hashcode(element) % hashset->nbuckets;
	//       2. Check if an equivalent element already exists
	//          in the corresponding bucket.
	int exists=0;
	exists=hashset_contains(hashset,element);
	//       3. If no equivalent element exists, create a copy of
	//          the element and store the copy in a new bucket node
	//          in the appropriate bucket.
	printf("exists is %d \n",exists);
	if(exists==0){
	    void* copy = hashset->copy(element);
	    BucketNode* bucket=((hashset->buckets)[bucket_number]);
	    BucketNode* new = malloc(sizeof(BucketNode));
	    new->value=copy;
	    new->next=NULL;
	    int i=0;
	    if(bucket==NULL){
		((hashset->buckets)[bucket_number])=new;
	    }else{
		while(bucket->next!=NULL){
		    bucket=bucket->next;
		}
		bucket->next=new;
	    }
	    hashset->size=(hashset->size)+1;
	    if(hashset->size > LOAD_FACTOR*hashset->nbuckets){
		BucketNode** temp =(BucketNode**)malloc(2*(hashset->nbuckets)*sizeof(BucketNode*));
		for(i=0;i<(hashset->nbuckets)*2;i++){
		    temp[i]=NULL;
		}
		for(i = 0; i<hashset->nbuckets; i++) { // go through all buckets
		    BucketNode *bn = hashset->buckets[i];
		    BucketNode* next = bn;
		    while(next) {
			if(next){
			    bn=next;
			    bucket_number = hashset->hashcode(next->value) % ((hashset->nbuckets)*2);
			    if(temp[bucket_number]==NULL){ //if nothing in new bucket
				next=bn->next;
				bn->next=NULL;
				temp[bucket_number]=bn;
			    }else{
				BucketNode* iter = temp[bucket_number];
				while(iter->next!=NULL){
				    iter=iter->next;
				}
				next=bn->next;
				bn->next=NULL;
				iter->next=bn;
				
			    }
			}
		    }
		}
		free(hashset->buckets);
		hashset->buckets=temp;
		hashset->nbuckets=(hashset->nbuckets)*2;
	    
	//       4. If the set size exceeds the load factor times
	//          the number of buckets, expand the bucket array and
	//          redistribute the existing nodes accordingly
	    }
	    return 1;
	}
	return 0;
}

/* Return 1 if the given element is in the set
 * 0 otherwise
 */
int hashset_contains(HashSet *hashset, void *element) {
	int bucket_number = hashset->hashcode(element) % hashset->nbuckets;
	BucketNode *current = hashset->buckets[bucket_number];
	while (current != NULL) {
		if (hashset->equals(current->value, element)) {
			return 1;
		}
		current = current->next;
	}
	return 0;
}

/* Remove the given element from the set
 * Return 1 if the element was removed successfully
 * 0 otherwise (if the element was not found)
 */
int hashset_remove(HashSet *hashset, void *element) {
	//printf("called on %s \n",element);
	// TODO: 1. Determine the bucket for the given element.
	int bucket_number = hashset->hashcode(element) % hashset->nbuckets;
	//       2. Check if an equivalent element already exists
	//          in the corresponding bucket.
	int exists=0;
	exists=hashset_contains(hashset,element);
	//       3. If an element is found, delete the hashset's copy
	//          of the element and remove the corresponding bucket node.
	if(exists==1){
 	    int index =0;
 	    BucketNode* bucket =(hashset->buckets)[bucket_number];
 	    while(hashset->equals(bucket->value,element)==0){
 		index++;
 		bucket=bucket->next;
 	    }
 	    if(index==0){ //head of the list
		hashset->delete(bucket->value);
		BucketNode* temp=(hashset->buckets)[bucket_number]->next;
		free((hashset->buckets)[bucket_number]);
		(hashset->buckets)[bucket_number]=temp;
 	    }else{ //not the head of the list
 		hashset->delete(bucket->value);
 		int x=0;
 		BucketNode* prev = (hashset->buckets)[bucket_number];
 		for(x=0;x<index-1;x++){
 		    prev=prev->next;
 		}
 		prev->next=bucket->next;
		free(bucket);
 	    }
 	     hashset->size=hashset->size-1;
	    return 1;
	}
	return 0;
}

/* Remove all elements from the set */
void hashset_clear(HashSet *hashset) {
	BucketNode *current,*next;
	int i;
	for (i = 0; i < hashset->nbuckets; i++) {
		current = hashset->buckets[i];
		while (current != NULL) {
			next = current->next;
			hashset_remove(hashset, current->value);
			current = next;
		}
		hashset->buckets[i] = NULL;
	}
}


/******************************
 * Other utility functions
 ******************************/

/* Get the size of the given set */
int hashset_size(HashSet *hashset) {
	return hashset->size;
}

/* Print a representation of the hashset,
 * using the given function to print each
 * element
 */
void hashset_print(HashSet *hashset, FILE *stream, void (*print_element)(FILE *, void *)) {
	fprintf(stream, "{size=%d, buckets=%d", hashset->size, hashset->nbuckets);
	int i;
	for(i = 0; i<hashset->nbuckets; i++) {
		fprintf(stream, ", %d=[", i);
		BucketNode *bn = hashset->buckets[i];
		while(bn) {
			print_element(stream, bn->value);
			bn = bn->next;
			if(bn)
				fprintf(stream, ", ");
		}
		fprintf(stream, "]");
	}
	fprintf(stream, "}");
}
