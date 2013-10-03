#include <assert.h>
#include "common.h"

static PacketNode headNode = {0}, tailNode = {0};
PacketNode * const head = &headNode, * const tail = &tailNode;

void initPacketNodeList() {
    if (head->next == NULL && tail->next == NULL) {
        // first time initializing
        head->next = tail;
        tail->prev = head;
    } else {
        // have used before, then check node is empty
        assert(isListEmpty());
    }
}

PacketNode* createNode(char* buf, UINT len, DIVERT_ADDRESS *addr) {
    PacketNode *newNode = (PacketNode*)malloc(sizeof(PacketNode));
    newNode->packet = (char*)malloc(len);
    memcpy(newNode->packet, buf, len);
    newNode->packetLen = len;
    memcpy(&(newNode->addr), addr, sizeof(DIVERT_ADDRESS));
    newNode->next = newNode->prev = NULL;
    return newNode;
}

void freeNode(PacketNode *node) {
    assert((node != head) && (node != tail));
    free(node->packet);
    free(node);
}

PacketNode* popNode(PacketNode *node) {
    assert((node != head) && (node != tail));
    node->prev->next = node->next;
    node->next->prev = node->prev;
    return node;
}

PacketNode* insertAfter(PacketNode *node, PacketNode *target) {
    node->prev = target;
    node->next = target->next;
    target->next->prev = node;
    target->next = node;
    return node;
}

PacketNode* insertBefore(PacketNode *node, PacketNode *target) {
    node->next = target;
    node->prev = target->prev;
    target->prev->next = node;
    target->prev = node;
    return node;
}

PacketNode* appendNode(PacketNode *node) {
    return insertBefore(node, tail);
}

short isListEmpty() {
    return head->next == tail;
}