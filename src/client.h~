#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include "serverDataModel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

extern char userId[74];
extern struct User* user;
extern struct UserList* contacts;

int addContact(char*);
int addUserToGroup(char*, char*);
int clearHistory(char*);
void cleanup();
char* createGroup(struct Group*);
void initialize();
struct UserList* getContacts(char*);
struct Group* getGroupInfo(char*);
struct MessageList* getMessages(char*);
struct User* getUser(char*);
struct GroupList* getUserGroups(char*);
struct User* login(char*);
int logout();
char* registerUser(struct User*);
int removeGroup(char*);
int removeMessage(struct Message*);
int removeUser();
int resendMessage(struct Message*);
char* sendMessage(struct Message*);