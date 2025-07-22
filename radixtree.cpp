#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_CHILDREN 256  // For ASCII characters

typedef struct RadixNode {
    char *key;                           // Compressed key segment
    void *value;                         // Value stored at this node (NULL if not a terminal)
    struct RadixNode *children[MAX_CHILDREN];  // Children array
    int num_children;                    // Number of active children
    bool is_terminal;                    // True if this node represents end of a key
} RadixNode;

typedef struct {
    RadixNode *root;
    int size;
} RadixTree;

// Function declarations
RadixTree* radix_create();
RadixNode* radix_node_create(const char *key);
void radix_node_free(RadixNode *node);
void radix_free(RadixTree *tree);
int radix_insert(RadixTree *tree, const char *key, void *value);
void* radix_search(RadixTree *tree, const char *key);
int radix_delete(RadixTree *tree, const char *key);
void radix_traverse(RadixTree *tree, void (*callback)(const char*, void*));
void radix_print(RadixTree *tree);

// Helper functions
static int find_common_prefix_length(const char *str1, const char *str2);
static RadixNode* radix_insert_recursive(RadixNode *node, const char *key, void *value, int *inserted);
static void* radix_search_recursive(RadixNode *node, const char *key);
static RadixNode* radix_delete_recursive(RadixNode *node, const char *key, int *deleted);
static void radix_traverse_recursive(RadixNode *node, char *prefix, int prefix_len, void (*callback)(const char*, void*));
static void radix_print_recursive(RadixNode *node, char *prefix, int prefix_len, int depth);

// Create a new radix tree
RadixTree* radix_create() {
    RadixTree *tree = (RadixTree*)malloc(sizeof(RadixTree));
    if (!tree) return NULL;
    
    tree->root = radix_node_create("");
    tree->size = 0;
    return tree;
}

// Create a new radix tree node
RadixNode* radix_node_create(const char *key) {
    RadixNode *node = (RadixNode*)malloc(sizeof(RadixNode));
    if (!node) return NULL;
    
    node->key = strdup(key);
    node->value = NULL;
    node->num_children = 0;
    node->is_terminal = false;
    
    for (int i = 0; i < MAX_CHILDREN; i++) {
        node->children[i] = NULL;
    }
    
    return node;
}

// Free a radix tree node and its subtree
void radix_node_free(RadixNode *node) {
    if (!node) return;
    
    for (int i = 0; i < MAX_CHILDREN; i++) {
        if (node->children[i]) {
            radix_node_free(node->children[i]);
        }
    }
    
    free(node->key);
    free(node);
}

// Free the entire radix tree
void radix_free(RadixTree *tree) {
    if (!tree) return;
    
    radix_node_free(tree->root);
    free(tree);
}

// Find the length of common prefix between two strings
static int find_common_prefix_length(const char *str1, const char *str2) {
    int i = 0;
    while (str1[i] && str2[i] && str1[i] == str2[i]) {
        i++;
    }
    return i;
}

// Insert a key-value pair into the radix tree
int radix_insert(RadixTree *tree, const char *key, void *value) {
    if (!tree || !key) return 0;
    
    int inserted = 0;
    tree->root = radix_insert_recursive(tree->root, key, value, &inserted);
    
    if (inserted) {
        tree->size++;
    }
    
    return inserted;
}

// Recursive helper for insertion
static RadixNode* radix_insert_recursive(RadixNode *node, const char *key, void *value, int *inserted) {
    if (!node) {
        node = radix_node_create(key);
        node->value = value;
        node->is_terminal = true;
        *inserted = 1;
        return node;
    }
    
    int common_len = find_common_prefix_length(node->key, key);
    int node_key_len = strlen(node->key);
    int key_len = strlen(key);
    
    if (common_len == node_key_len) {
        // The node's key is a prefix of the search key
        if (common_len == key_len) {
            // Exact match - update value
            if (!node->is_terminal) {
                node->is_terminal = true;
                *inserted = 1;
            }
            node->value = value;
            return node;
        } else {
            // Continue with the remaining key
            const char *remaining_key = key + common_len;
            unsigned char first_char = (unsigned char)remaining_key[0];
            
            node->children[first_char] = radix_insert_recursive(
                node->children[first_char], remaining_key, value, inserted
            );
            
            if (node->children[first_char] && !node->children[first_char]->children[first_char]) {
                node->num_children++;
            }
            
            return node;
        }
    } else {
        // Need to split the node
        RadixNode *new_node = radix_node_create(node->key + common_len);
        new_node->value = node->value;
        new_node->is_terminal = node->is_terminal;
        new_node->num_children = node->num_children;
        
        // Move children to new node
        for (int i = 0; i < MAX_CHILDREN; i++) {
            new_node->children[i] = node->children[i];
            node->children[i] = NULL;
        }
        
        // Update current node
        char *old_key = node->key;
        node->key = (char*)malloc(common_len + 1);
        strncpy(node->key, old_key, common_len);
        node->key[common_len] = '\0';
        free(old_key);
        
        node->value = NULL;
        node->is_terminal = false;
        node->num_children = 1;
        
        // Add the split-off part as a child
        unsigned char first_char = (unsigned char)new_node->key[0];
        node->children[first_char] = new_node;
        
        // Insert the new key
        if (common_len == key_len) {
            node->value = value;
            node->is_terminal = true;
            *inserted = 1;
        } else {
            const char *remaining_key = key + common_len;
            unsigned char new_first_char = (unsigned char)remaining_key[0];
            
            node->children[new_first_char] = radix_insert_recursive(
                NULL, remaining_key, value, inserted
            );
            node->num_children++;
        }
        
        return node;
    }
}

// Search for a key in the radix tree
void* radix_search(RadixTree *tree, const char *key) {
    if (!tree || !key) return NULL;
    
    return radix_search_recursive(tree->root, key);
}

// Recursive helper for search
static void* radix_search_recursive(RadixNode *node, const char *key) {
    if (!node) return NULL;
    
    int common_len = find_common_prefix_length(node->key, key);
    int node_key_len = strlen(node->key);
    int key_len = strlen(key);
    
    if (common_len == node_key_len) {
        if (common_len == key_len) {
            return node->is_terminal ? node->value : NULL;
        } else {
            const char *remaining_key = key + common_len;
            unsigned char first_char = (unsigned char)remaining_key[0];
            return radix_search_recursive(node->children[first_char], remaining_key);
        }
    }
    
    return NULL;
}

// Delete a key from the radix tree
int radix_delete(RadixTree *tree, const char *key) {
    if (!tree || !key) return 0;
    
    int deleted = 0;
    tree->root = radix_delete_recursive(tree->root, key, &deleted);
    
    if (deleted) {
        tree->size--;
    }
    
    return deleted;
}

// Recursive helper for deletion
static RadixNode* radix_delete_recursive(RadixNode *node, const char *key, int *deleted) {
    if (!node) return NULL;
    
    int common_len = find_common_prefix_length(node->key, key);
    int node_key_len = strlen(node->key);
    int key_len = strlen(key);
    
    if (common_len == node_key_len) {
        if (common_len == key_len) {
            // Found the node to delete
            if (node->is_terminal) {
                node->is_terminal = false;
                node->value = NULL;
                *deleted = 1;
                
                // If node has no children, it can be removed
                if (node->num_children == 0) {
                    radix_node_free(node);
                    return NULL;
                }
                
                // If node has only one child, merge with child
                if (node->num_children == 1) {
                    RadixNode *child = NULL;
                    for (int i = 0; i < MAX_CHILDREN; i++) {
                        if (node->children[i]) {
                            child = node->children[i];
                            break;
                        }
                    }
                    
                    // Merge node with its single child
                    char *new_key = (char*)malloc(strlen(node->key) + strlen(child->key) + 1);
                    strcpy(new_key, node->key);
                    strcat(new_key, child->key);
                    
                    free(node->key);
                    free(child->key);
                    
                    node->key = new_key;
                    node->value = child->value;
                    node->is_terminal = child->is_terminal;
                    node->num_children = child->num_children;
                    
                    for (int i = 0; i < MAX_CHILDREN; i++) {
                        node->children[i] = child->children[i];
                    }
                    
                    free(child);
                }
            }
            return node;
        } else {
            // Continue deletion in subtree
            const char *remaining_key = key + common_len;
            unsigned char first_char = (unsigned char)remaining_key[0];
            
            RadixNode *old_child = node->children[first_char];
            node->children[first_char] = radix_delete_recursive(
                node->children[first_char], remaining_key, deleted
            );
            
            if (old_child && !node->children[first_char]) {
                node->num_children--;
            }
            
            // Check if current node can be merged or removed
            if (!node->is_terminal && node->num_children == 1) {
                RadixNode *child = NULL;
                for (int i = 0; i < MAX_CHILDREN; i++) {
                    if (node->children[i]) {
                        child = node->children[i];
                        break;
                    }
                }
                
                // Merge node with its single child
                char *new_key = (char*)malloc(strlen(node->key) + strlen(child->key) + 1);
                strcpy(new_key, node->key);
                strcat(new_key, child->key);
                
                free(node->key);
                free(child->key);
                
                node->key = new_key;
                node->value = child->value;
                node->is_terminal = child->is_terminal;
                node->num_children = child->num_children;
                
                for (int i = 0; i < MAX_CHILDREN; i++) {
                    node->children[i] = child->children[i];
                }
                
                free(child);
            }
            
            return node;
        }
    }
    
    return node;
}

// Traverse the radix tree and call callback for each key-value pair
void radix_traverse(RadixTree *tree, void (*callback)(const char*, void*)) {
    if (!tree || !callback) return;
    
    char prefix[1000];  // Assume keys won't exceed 1000 characters
    radix_traverse_recursive(tree->root, prefix, 0, callback);
}

// Recursive helper for traversal
static void radix_traverse_recursive(RadixNode *node, char *prefix, int prefix_len, void (*callback)(const char*, void*)) {
    if (!node) return;
    
    // Add current node's key to prefix
    int key_len = strlen(node->key);
    strcpy(prefix + prefix_len, node->key);
    int new_prefix_len = prefix_len + key_len;
    
    // If this is a terminal node, call callback
    if (node->is_terminal) {
        prefix[new_prefix_len] = '\0';
        callback(prefix, node->value);
    }
    
    // Recurse on children
    for (int i = 0; i < MAX_CHILDREN; i++) {
        if (node->children[i]) {
            radix_traverse_recursive(node->children[i], prefix, new_prefix_len, callback);
        }
    }
}

// Print the radix tree structure
void radix_print(RadixTree *tree) {
    if (!tree) return;
    
    printf("Radix Tree (size: %d):\n", tree->size);
    char prefix[1000];
    radix_print_recursive(tree->root, prefix, 0, 0);
}

// Recursive helper for printing tree structure
static void radix_print_recursive(RadixNode *node, char *prefix, int prefix_len, int depth) {
    if (!node) return;
    
    // Print indentation
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
    
    // Add current node's key to prefix
    int key_len = strlen(node->key);
    strcpy(prefix + prefix_len, node->key);
    int new_prefix_len = prefix_len + key_len;
    prefix[new_prefix_len] = '\0';
    
    // Print node information
    if (node->is_terminal) {
        printf("'%s' -> %p (terminal)\n", prefix, node->value);
    } else {
        printf("'%s' (internal)\n", node->key);
    }
    
    // Recurse on children
    for (int i = 0; i < MAX_CHILDREN; i++) {
        if (node->children[i]) {
            radix_print_recursive(node->children[i], prefix, new_prefix_len, depth + 1);
        }
    }
}

// Example callback function for traversal
void print_key_value(const char *key, void *value) {
    printf("Key: '%s', Value: %p\n", key, value);
}

// Example usage and test function
int main() {
    RadixTree *tree = radix_create();
    
    // Test data
    char *keys[] = {"hello", "help", "hell", "world", "word", "work", "test", "testing", "tea", "team"};
    int values[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int num_keys = sizeof(keys) / sizeof(keys[0]);
    
    printf("=== Radix Tree Test ===\n\n");
    
    // Insert keys
    printf("Inserting keys:\n");
    for (int i = 0; i < num_keys; i++) {
        int result = radix_insert(tree, keys[i], &values[i]);
        printf("Insert '%s': %s\n", keys[i], result ? "SUCCESS" : "FAILED");
    }
    printf("\n");
    
    // Print tree structure
    radix_print(tree);
    printf("\n");
    
    // Search for keys
    printf("Searching for keys:\n");
    for (int i = 0; i < num_keys; i++) {
        void *result = radix_search(tree, keys[i]);
        if (result) {
            printf("Search '%s': FOUND (value: %d)\n", keys[i], *(int*)result);
        } else {
            printf("Search '%s': NOT FOUND\n", keys[i]);
        }
    }
    
    // Search for non-existent key
    printf("Search 'nonexistent': %s\n", radix_search(tree, "nonexistent") ? "FOUND" : "NOT FOUND");
    printf("\n");
    
    // Traverse tree
    printf("Tree traversal:\n");
    radix_traverse(tree, print_key_value);
    printf("\n");
    
    // Delete some keys
    printf("Deleting keys:\n");
    char *keys_to_delete[] = {"help", "test", "word"};
    int num_delete = sizeof(keys_to_delete) / sizeof(keys_to_delete[0]);
    
    for (int i = 0; i < num_delete; i++) {
        int result = radix_delete(tree, keys_to_delete[i]);
        printf("Delete '%s': %s\n", keys_to_delete[i], result ? "SUCCESS" : "FAILED");
    }
    printf("\n");
    
    // Print tree after deletion
    printf("Tree after deletion:\n");
    radix_print(tree);
    printf("\n");
    
    // Final traversal
    printf("Final tree traversal:\n");
    radix_traverse(tree, print_key_value);
    
    // Cleanup
    radix_free(tree);
    
    return 0;
}