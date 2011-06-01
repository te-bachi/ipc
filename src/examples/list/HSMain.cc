
void *socketThread(void *param) {
    List               *list;
    ListIter           *iter;
    int                 listenfd;
    int                 connectfd;
    struct sockaddr_in  serverAddr;
    struct sockaddr_in  clientAddr;
    pthread_t           tid;
    
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) >= 0) {
        bzero(&serverAddr, sizeof(serverAddr));
        serverAddr.sin_family      = AF_INET;
        serverAddr.sin.addr.s_addr = htonl(INADDR_ANY);
        serverAddr.sin_port        = htons(COMM_PORT);
        
        if (bind(listenfd, (SA *) &serverAddr, sizeof(serverAddr)) == 0) {
            if (listen(listenfd, SENSOR_MAX_NUM) == 0) {
                list = list_new();
                while (running) {
                    if ((clientAddr = accept(listenfd, (SA *) clientAddr, sizeof(clientAddr))) >= 0) {
                        
                    } else {
                        debug(ERROR, "Can't accept connection: %s", strerror(errno));
                    }
                }
                
                iter = list_iter_new(list);
                while ((tid = (pthread_t) list_iter_next(iter)) != NULL) {
                    pthread_join(tid, NULL);
                }
                list_iter_free(iter);
                list_free(list);
            } else {
                debug(FATAL, "Can't mark socket as passive (listen-mode): %s", strerror(errno));
                shutdown();
            }
        } else {
            debug(FATAL, "Can't bind socket: %s", strerror(errno));
            shutdown();
        }
        
        close(listenfd);
        
    } else {
        debug(FATAL, "Can't create socket: %s", strerror(errno));
        shutdown();
    }
    
    return NULL;
}
