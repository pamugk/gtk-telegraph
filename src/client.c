#include "client.h"

int mainSocket,notifierSocket;
char* userId;
struct User* user = NULL;
struct UserList* contacts = NULL;
struct GroupList* groups = NULL;

void cleanup() {
	if (user != NULL) {
		logout();
		userDestructor (user);
	}
	if (contacts != NULL)
		userListDestructor(contacts);
	if (groups != NULL)
		groupDestructor (groups);
}

struct sockaddr_in setupServer() {
    printf("Setting server up.\n");
    FILE* settingsFile = fopen("settings.txt", "r");
    if (settingsFile == NULL) {
        perror("fopen");
        exit(1);
    }
    char serverIp[16];
    int serverPort;
    int res = fscanf(settingsFile, "%s%d", serverIp, &serverPort);
    if (res == -1) {
        perror("fscanf");
        fclose(settingsFile);
        exit(1);
    }
    fclose(settingsFile);
    struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(serverIp);
	server.sin_port = htons(serverPort);
    printf("Done.\n");
    return server;
}

int initialize() {
	struct sockaddr_in server = setupServer();
    mainSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (mainSocket == -1) {
		perror("socket error");
		return 1;
	}
    notifierSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (notifierSocket == -1) {
		perror("socket error");
		return 1;
	}
	if (connect(mainSocket, (struct sockaddr *) &server, sizeof(server)) == -1) {
		perror("connect error");
		return 1;
	}
	if (connect(notifierSocket, (struct sockaddr *) &server, sizeof(server)) == -1) {
		perror("connect error");
        close(mainSocket);
		return 1;
	}
	return 0;
}

#pragma region Callbacks
struct Message* doRecieveMessage(int nsock, int recieveHeaderOnly);

void* callback(void* params) {
    enum ServerNotifications notification;
	ssize_t size;
    do {
		size = recv(notifierSocket, &notification, sizeof(enum ServerNotifications), 0);
		if (size == -1) {
			perror("recv");
			break;
		}
        printf("Got a new notification\n");
        switch (notification)
        {
        case NEW_MESSAGE:{
            struct Message* message = doRecieveMessage(notifierSocket, 0);
			acceptNewMessage(message);
            break;
        }
        case SHUTDOWN:{
	        printf("Server is down, long live the server.\n");
			onServerShutdown();
            break;
        }
        }
    } while (notification != SHUTDOWN);
    close (notifierSocket);
    pthread_exit(0);
}

void setupCallbacks() {
	pthread_t tid;
	int res = pthread_create(&tid, NULL, callback, NULL);
	if (res == -1)
		perror("pthread_create");
}
#pragma endregion
#pragma region Destructors
void charDestructor(struct CharList* strings) {
    for (int i = 0; i < strings->count; i += 1)
        free(strings->list[i]);
    free(strings->list);
}

void groupDestructor(struct Group* group) {
    if (group->id != NULL)
        free(group->id);
    free(group->creatorId);
    for (int i = 0; i < group->countOfParticipants; i += 1)
        free(group->participants[i]);
    free(group->participants);
}

void groupListDestructor(struct GroupList* groupList) {
    for (int i = 0; i < groupList->count; i += 1)
        groupDestructor(groupList->list[i]);
    free(groupList->list);
}

void messageDestructor(struct Message* message) {
    if (message->id != NULL)
        free(message->id);
    free(message->toId);
    free(message->fromId);
    if (message->text != NULL)
        free(message->text);
}

void messageListDestructor(struct MessageList* messageList) {
    for (int i = 0; i < messageList->count; i += 1)
        messageDestructor(messageList->list[i]);
    free(messageList->list);
}

void userDestructor(struct User* user) {
    if (user->id != NULL)
        free(user->id);
    free(user->phone);
    free(user->username);
    free(user->name);
    free(user->surname);
	free(user->fullname);
    if (user->biography != NULL)
        free(user->biography);
}

void userListDestructor(struct UserList* userList) {
    for (int i = 0; i < userList->count; i += 1)
        userDestructor(userList->list[i]);
    free(userList->list);
}
#pragma endregion
#pragma region Auxillary functions
char* doRecieveStr(int mainSocket) {
    int8_t isNull;
	int res = recv(mainSocket, &isNull, sizeof(int8_t), 0);
    char* str = NULL;
	if (isNull != 0) {
        size_t size;
        recv(mainSocket, &size, sizeof(size_t), 0);
        str = calloc(size, sizeof(char));
        recv(mainSocket, str, size * sizeof(char), 0);
	}
    return str;
}

struct CharList* doRecieveStrs(int nsock) {
	struct CharList* strings = (struct CharList*)malloc(sizeof(struct CharList));
	recv(nsock, &(strings->count), sizeof(int), 0);
	for (int i = 0; i < strings->count; i+= 1)
		strings->list[i] = doRecieveStr(nsock);
	return strings;
}

struct Group* doRecieveGroup(int nsock) {
    struct Group* group = (struct Group*) malloc(sizeof(struct Group));
	group->id = doRecieveStr(nsock);
	group->creatorId = doRecieveStr(nsock);
	group->name = doRecieveStr(nsock);
    recv(nsock, &group->countOfParticipants, sizeof(int), 0);
    for (int i = 0; i < group->countOfParticipants; i += 1)
        group->participants[i] = doRecieveStr(nsock);
	return group;
}

struct GroupList* doRecieveGroups(int nsock) {
    struct GroupList* groups = (struct GroupList*) malloc(sizeof(struct GroupList));
    recv(nsock, &groups->count, sizeof(int), 0);
    groups->list = (struct Group**)calloc(groups->count, sizeof(struct Group*));
    for (int i = 0; i < groups->count; i += 1)
        groups->list[i] = doRecieveGroup(nsock);
	return groups;
}

struct Message* doRecieveMessage(int nsock, int recieveHeaderOnly) {
	struct Message* message = (struct Message*) malloc(sizeof(struct Message));
	message->id = doRecieveStr(nsock);
	message->fromId = doRecieveStr(nsock);
	message->toId = doRecieveStr(nsock);
    if (recieveHeaderOnly == 0)
	    message->text = doRecieveStr(nsock);
	return message;
}

struct MessageList* doRecieveMessages(int nsock) {
    struct MessageList* messages = (struct MessageList*) malloc(sizeof(struct MessageList));
    recv(nsock, &messages->count, sizeof(int), 0);
    messages->list = (struct Message**)calloc(messages->count, sizeof(struct Message*));
    for (int i = 0; i < messages->count; i += 1)
        messages->list[i] = doRecieveMessage(nsock, 0);
	return messages;
}

struct User* doRecieveUser(int mainSocket) {
    struct User* user = (struct User*) malloc(sizeof(struct User));
    user->id = doRecieveStr(mainSocket);
    user->phone = doRecieveStr(mainSocket);
    user->username = doRecieveStr(mainSocket);
    user->name = doRecieveStr(mainSocket);
    user->surname = doRecieveStr(mainSocket);
	user->fullname = (char*)calloc(strlen(user->name) + 
		strlen(user->surname) + 1, sizeof(char));
	sprintf(user->fullname, "%s %s", user->name, user->surname);
    user->biography = doRecieveStr(mainSocket);
    return user;
}

struct UserList* doRecieveUsers(int nsock) {
    struct UserList* users = (struct UserList*) malloc(sizeof(struct UserList));
    recv(nsock, &(users->count), sizeof(int), 0);
    users->list = (struct User**)calloc(users->count, sizeof(struct User*));
    for (int i = 0; i < users->count; i += 1)
        users->list[i] = doRecieveUser(nsock);
	return users;
}

int doSendStr(int nsock, char* str) {
    int res = 0;
    int8_t isNull = str == NULL ? 0 : 1;
	res = send(nsock, &isNull, sizeof(int8_t), 0);
	if (isNull != 0) {
		size_t size = strlen(str) + 1;
		res = send(nsock, &size, sizeof(size_t), 0);
        if (res == -1){
            perror("send");
            return 1;
        }
		res = send(nsock, str, size * sizeof(char), 0);
        if (res == -1){
            perror("send");
            return 1;
        }
	}
	return 0;
}

int doSendGroup(int nsock, struct Group* group) {
	doSendStr(nsock, group->id);
	doSendStr(nsock, group->creatorId);
	doSendStr(nsock, group->name);
    send(nsock, &group->countOfParticipants, sizeof(int), 0);
    for (int i = 0; i < group->countOfParticipants; i += 1)
        doSendStr(nsock, group->participants[i]);
	return 0;
}

int doSendMessage(int nsock, struct Message* message, int sendHeaderOnly) {
	doSendStr(nsock, message->id);
	doSendStr(nsock, message->fromId);
	doSendStr(nsock, message->toId);
    if (sendHeaderOnly == 0)
	    doSendStr(nsock, message->text);
	return 0;
}

int doSendUser(int nsock, struct User* user) {
	doSendStr(nsock, user->id);
	doSendStr(nsock, user->phone);
	doSendStr(nsock, user->username);
	doSendStr(nsock, user->name);
	doSendStr(nsock, user->surname);
	doSendStr(nsock, user->biography);
	return 0;
}
#pragma endregion
#pragma region Client functions
int addContact(char* contactId) {
    printf("Adding contact.\n");
    enum ServerOperations operation = ADD_CONTACT;
    send(mainSocket, &operation, sizeof(enum ServerOperations), 0);
    doSendStr(mainSocket, contactId);
    enum ServerResponses response;
    int res = recv(mainSocket, &response, sizeof(enum ServerResponses), 0);
    if (response == SUCCESS) {
        printf("Done.\n");
        return 0;
    }
    printf("Failure\n");
    return 1;
}

int addUserToGroup(char* groupId, char* userId) {
    printf("Adding user to group.\n");
    enum ServerOperations operation = ADD_USER_TO_GROUP;
    send(mainSocket, &operation, sizeof(enum ServerOperations), 0);
    doSendStr(mainSocket, groupId);
    doSendStr(mainSocket, userId);
    enum ServerResponses response;
    int res = recv(mainSocket, &response, sizeof(enum ServerResponses), 0);
    if (response == SUCCESS) {
        printf("Done.\n");
        return 0;
    }
    printf("Failure\n");
    return 1;
}

int clearHistory(char* fromId) {
    printf("Clearing history of messages.\n");
    enum ServerOperations operation = CLEAR_HISTORY;
    send(mainSocket, &operation, sizeof(enum ServerOperations), 0);
    doSendStr(mainSocket, fromId);
    enum ServerResponses response;
    int res = recv(mainSocket, &response, sizeof(enum ServerResponses), 0);
    if (response == SUCCESS) {
        printf("Done.\n");
        return 0;
    }
    printf("Failure\n");
    return 1;
}

char* createGroup(struct Group* newGroup) {
    printf("Asking for the creation of a new group.\n");
    enum ServerOperations operation = CREATE_GROUP;
    int res = send(mainSocket, &operation, sizeof(enum ServerOperations), 0);
    doSendGroup(mainSocket, newGroup);
    enum ServerResponses response;
    res = recv(mainSocket, &response, sizeof(enum ServerResponses), 0);
    char* createdGroupId = NULL;
    if (response == SUCCESS) {
        createdGroupId = doRecieveStr(mainSocket);
        printf("Done.\n");
    }
    else
        printf("Failure.\n");
    return createdGroupId;
}

struct UserList* getContacts() {
    printf("Asking for the contacts.\n");
    enum ServerOperations operation = GET_CONTACTS;
    int res = send(mainSocket, &operation, sizeof(enum ServerOperations), 0);
    if (res == -1) {
        perror("send");
        exit(1);
    }
    enum ServerResponses response;
    res = recv(mainSocket, &response, sizeof(enum ServerResponses), 0);
    struct UserList* contacts = NULL;
    if (res == -1) {
        perror("recv");
        return contacts;
    }
    if (response == SUCCESS) {
        contacts = doRecieveUsers(mainSocket);
        printf("Done.\n");
    }
    else
        printf("Failure\n");
    return contacts;
}

struct Group* getGroupInfo(char* groupId) {
    printf("Fetching group.\n");
    enum ServerOperations operation = GET_GROUP_INFO;
    int res = send(mainSocket, &operation, sizeof(enum ServerOperations), 0);
    res = doSendStr(mainSocket, groupId);
    enum ServerResponses response;
    res = recv(mainSocket, &response, sizeof(enum ServerResponses), 0);
    struct Group* group = NULL;
    if (response == SUCCESS) {
        group = doRecieveGroup(mainSocket);
        printf("Done.\n");
    }
    else
        printf("Failure\n");
    return group;
}

struct MessageList* getMessages(char* fromId) {
    printf("Fetching messages.\n");
    enum ServerOperations operation = GET_MESSAGES;
    int res = send(mainSocket, &operation, sizeof(enum ServerOperations), 0);
    res = doSendStr(mainSocket, fromId);
    enum ServerResponses response;
    res = recv(mainSocket, &response, sizeof(enum ServerResponses), 0);
    struct MessageList* messages = NULL;
    if (response == SUCCESS) {
        messages = doRecieveMessages(mainSocket);
        printf("Done.\n");
    }
    else
        printf("Failure\n");
    return messages;
}

struct User* getUser(char* userId) {
    printf("Fetching user.\n");
    enum ServerOperations operation = GET_USER;
    int res = send(mainSocket, &operation, sizeof(enum ServerOperations), 0);
    res = doSendStr(mainSocket, userId);
    enum ServerResponses response;
    res = recv(mainSocket, &response, sizeof(enum ServerResponses), 0);
    struct User* user = NULL;
    if (response == SUCCESS) {
        user = doRecieveUser(mainSocket);
        printf("Done.\n");
    }
    else
        printf("Failure\n");
    return user;
}

struct GroupList* getUserGroups() {
    printf("Asking for a groups.\n");
    enum ServerOperations operation = GET_USER_GROUPS;
    int res = send(mainSocket, &operation, sizeof(enum ServerOperations), 0);
    enum ServerResponses response;
    res = recv(mainSocket, &response, sizeof(enum ServerResponses), 0);
    struct GroupList* groups = NULL;
    if (response == SUCCESS) {
        struct CharList* groupIds = doRecieveStrs(mainSocket);
		groups = (struct GroupList*)malloc(sizeof(struct GroupList));
		groups->count = groupIds->count;
		groups->list = (struct Group**)calloc(groups->count, sizeof(struct Group*));
		for (int i = 0; i < groups->count; i += 1) {
			groups->list[i] = getGroupInfo (groupIds->list[i]);
			if (groups->list[i] != NULL)
				continue;
			charDestructor(groupIds);
			free(groupIds);
			i -= 1;
			while (i >= 0) {
				groupDestructor (groups->list[i]);
				i -= 1;
			}
			free(groups->list);
			free(groups);
        	printf("Failure\n");
			return NULL;
		}
        printf("Done.\n");
    }
    else
        printf("Failure\n");
    return groups;
}

struct User* login(char* passedUserId) {
    printf("Logging in.\n");
    enum ServerOperations operation = LOGIN;
    int res = send(mainSocket, &operation, sizeof(enum ServerOperations), 0);
    res = doSendStr(mainSocket, passedUserId);
    enum ServerResponses response;
    res = recv(mainSocket, &response, sizeof(enum ServerResponses), 0);
    struct User* user = NULL;
    if (response == SUCCESS) {
		userId = passedUserId;
        setupCallbacks();
    	user = doRecieveUser(mainSocket);
        printf("Done.\n");
    }
    else
        printf("Failure\n");
    return user;
}

int logout() {
    printf("Logging out.\n");
    enum ServerOperations operation = DISCONNECT;
    send(mainSocket, &operation, sizeof(enum ServerOperations), 0);
    close(mainSocket);
    printf("Done.\n");
	return 0;
}

char* registerUser(struct User* user) {
    printf("Asking for the registration of a new user.\n");
    enum ServerOperations operation = REGISTER_USER;
    int res = send(mainSocket, &operation, sizeof(enum ServerOperations), 0);
    doSendUser(mainSocket, user);
    enum ServerResponses response;
    res = recv(mainSocket, &response, sizeof(enum ServerResponses), 0);
    char* registeredUserId = NULL;
    if (response == SUCCESS) {
        registeredUserId = doRecieveStr(mainSocket);
        printf("Done.\n");
    }
    else
        printf("Failure.\n");
    return registeredUserId;
}

int removeGroup(char* groupId) {
    printf("Asking for the removal of a group.\n");
    enum ServerOperations operation = REMOVE_GROUP;
    int res = send(mainSocket, &operation, sizeof(enum ServerOperations), 0);
    res = doSendStr(mainSocket, groupId);
    enum ServerResponses response;
    res = recv(mainSocket, &response, sizeof(enum ServerResponses), 0);
    if (response == SUCCESS) {
        printf("Done.\n");
        return 0;
    }
    printf("Failure.\n");
    return 1;
}

int removeMessage(struct Message* message) {
    printf("Asking for the removal of a message.\n");
    enum ServerOperations operation = REMOVE_MESSAGE;
    int res = send(mainSocket, &operation, sizeof(enum ServerOperations), 0);
    doSendMessage(mainSocket, message, 1);
    enum ServerResponses response;
    res = recv(mainSocket, &response, sizeof(enum ServerResponses), 0);
    if (response == SUCCESS) {
        printf("Done.\n");
        return 0;
    }
    printf("Failure.\n");
    return 1;
}

int removeUser() {
    printf("Asking for the removal of a user.\n");
    enum ServerOperations operation = REMOVE_USER;
    int res = send(mainSocket, &operation, sizeof(enum ServerOperations), 0);
    enum ServerResponses response;
    res = recv(mainSocket, &response, sizeof(enum ServerResponses), 0);
    if (response == SUCCESS) {
        printf("Done.\n");
        return 0;
    }
    printf("Failure.\n");
    return 1;
}

int resendMessage(struct Message* message) {
    printf("Asking for the resending of a message.\n");
    enum ServerOperations operation = RESEND_MESSAGE;
    int res = send(mainSocket, &operation, sizeof(enum ServerOperations), 0);
    doSendMessage(mainSocket, message, 1);
    enum ServerResponses response;
    res = recv(mainSocket, &response, sizeof(enum ServerResponses), 0);
    if (response == SUCCESS) {
        printf("Done.\n");
        return 0;
    }
    printf("Failure.\n");
    return 1;
}

char* sendMessage(struct Message* newMessage) {
    printf("Asking for the sending of a message.\n");
    enum ServerOperations operation = SEND_MESSAGE;
    int res = send(mainSocket, &operation, sizeof(enum ServerOperations), 0);
    doSendMessage(mainSocket, newMessage, 0);
    enum ServerResponses response;
    res = recv(mainSocket, &response, sizeof(enum ServerResponses), 0);
    if (response == SUCCESS) {
        char* messageId = doRecieveStr(mainSocket);
        printf("Done.\n");
        return messageId;
    }
	else
    	printf("Failure.\n");
    return NULL;
}
#pragma endregion
