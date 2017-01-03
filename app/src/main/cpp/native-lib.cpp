#include <jni.h>
#include <string>
#include<stdio.h>
#include<stdarg.h>
#include <errno.h>
#include<string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stddef.h>

#define MAX_LOG_MESSAGE_LENGTH 256
#define MAX_BUFFER_SIZE 80

static void LogMessage(JNIEnv* env,jobject obj,const char *format,...){
    static jmethodID  methodID=NULL;
    if(NULL==methodID){
        jclass  clazz=env->GetObjectClass(obj);
        env->GetMethodID(clazz,"logMessage","(Ljava/lang/String;)V");
        env->DeleteGlobalRef(clazz);
    }
    if(NULL!=methodID){
        char buffer[MAX_LOG_MESSAGE_LENGTH];
        va_list ap;
        va_start(ap,format);
        vsnprintf(buffer,MAX_LOG_MESSAGE_LENGTH,format,ap);
        va_end(ap);
        jstring  message=  env->NewStringUTF(buffer);
        if(NULL!=message){
            env->CallVoidMethod(obj,methodID,message);
            env->DeleteGlobalRef(message);
        }
    }
}
static void ThrowException(JNIEnv*env,const char*className,const char*message){
    jclass clazz=env->FindClass(className);
    if(NULL!=clazz){
        env->ThrowNew(clazz,message);
        env->DeleteGlobalRef(clazz);
    }
}
static void ThrowErrnoException(JNIEnv*env,const char *className,int errnum){
    char buffer[MAX_LOG_MESSAGE_LENGTH];
    if(-1==strerror_r(errnum,buffer,MAX_LOG_MESSAGE_LENGTH)){
        strerror_r(errno,buffer,MAX_LOG_MESSAGE_LENGTH);
    }
    ThrowException(env,className,buffer);
}

static int NewTcpSocket(JNIEnv*env,jobject obj){
    LogMessage(env,obj,"Constructing a new TCP socket...");
    int tcpSocket=socket(PF_INET,SOCK_STREAM,0);
    if(-1==tcpSocket){
        ThrowErrnoException(env,"java/io/IOException",errno);
    }
   return tcpSocket;
}
static void BindSocketToPort(JNIEnv *env,jobject obj,int sd, unsigned short port){
    struct sockaddr_in address;
    memset(&address,0,sizeof(address));
    address.sin_family=PF_INET;
    address.sin_addr.s_addr=htonl(INADDR_ANY);
    address.sin_port=htons(port);
    LogMessage(env,obj,"Binding to port %hu.",port);
    if(-1==bind(sd,(struct sockaddr*)&address, sizeof(address))){
        ThrowErrnoException(env,"java/io/IOException",errno);
    }
}
static unsigned short GetSocketPort(JNIEnv*env,jobject obj,int sd){
    unsigned short port=0;
    struct sockaddr_in address;
    socklen_t  addressLength= sizeof(address);
    if(-1==getsockname(sd,(struct sockaddr*)&address,&addressLength)){
        ThrowErrnoException(env,"java/io/IOException",errno);
    }
    else
    {
        port=ntohs(address.sin_port);
        LogMessage(env,obj,"Bind to random port %hu.",port);
    }
    return port;
}
static void ListenOnSocket(JNIEnv*env,jobject obj,int sd,int backlog){
    LogMessage(env,obj,"Listening on socket with a backlog of %d pending connections.");
    if(-1==listen(sd,backlog)){
        ThrowErrnoException(env,"java/io/IOException",errno);
    }
}
static void LogAddress(JNIEnv*env,jobject obj,const char *message,const struct sockaddr_in *address){
    char ip[INET_ADDRSTRLEN];
    if(NULL==inet_ntop(PF_INET,&(address->sin_addr),ip,INET_ADDRSTRLEN)){
        ThrowErrnoException(env,"java/io/Exception",errno);
    }
    else{
        unsigned short port=ntohs(address->sin_port);
        LogMessage(env,obj,"%s %s :%hu.",message,ip,port);
    }
}
static int AcceptOnSocket(JNIEnv*env,jobject obj,int sd){
   struct sockaddr_in address;
    socklen_t addressLength= sizeof(address);
    LogMessage(env,obj,"Wating for a client connection...");
    int clientSocket=accept(sd,(struct sockaddr*)&address,&addressLength);
    if(-1==clientSocket){
        ThrowErrnoException(env,"java/io/IOException",errno);
    } else{
        LogAddress(env,obj,"Client connection from",&address);
    }
    return clientSocket;
}
static ssize_t  ReceiveFromSocket(JNIEnv* env,jobject obj,int sd,char *buffer,size_t bufferSize){
    LogMessage(env,obj,"Receiving from the socket....");
    ssize_t  recvSize=recv(sd,buffer,bufferSize-1,0);
    if(-1==recvSize){
        ThrowErrnoException(env,"java/io/IOException",errno);
    } else{
        buffer[recvSize]=NULL;
        if(recvSize>0){
            LogMessage(env,obj,"Received %d bytes: %s",recvSize,buffer);
        } else{
            LogMessage(env,obj,"Client disconnected.");
        }
        return recvSize;
    }
}
static ssize_t  SendToSocket(JNIEnv*env,jobject obj,int sd,const char *buffer,size_t bufferSize){
    LogMessage(env,obj,"Sending to the socket...");
    ssize_t  sentSize=send (sd,buffer,bufferSize,0);
    if (-1 == sentSize)
    {
        ThrowErrnoException(env, "java/io/IOException", errno);
    }
    else
    {
        if (sentSize > 0)
        {
            LogMessage(env, obj, "Sent %d bytes: %s", sentSize, buffer);
        }
        else
        {
            LogMessage(env, obj, "Client disconnected.");
        }
    }

    return sentSize;
}
static void ConnectToAddress(JNIEnv *env,jobject obj,int sd,const char *ip, unsigned short port){
    LogMessage(env,obj,"Connecting to %s:%un...",ip,port);
    struct sockaddr_in address;
    memset(&address,0, sizeof(address));
    address.sin_family=PF_INET;
    if(0==inet_aton(ip,&(address.sin_addr))){
        ThrowErrnoException(env,"java/io/IOException",errno);
    } else{
        address.sin_port=htons(port);
        if(-1==connect(sd,(const sockaddr*)&address, sizeof(address))){
            ThrowErrnoException(env,"java/io/IOException",errno);
        } else{
            LogMessage(env,obj,"Connected.");
        }
    }

}
static int NewUdpSocket(JNIEnv*env,jobject obj){
    LogMessage(env,obj,"Constructing a new UDP socket...");
    int udpSocket=socket(PF_INET,SOCK_DGRAM,0);
    if(-1==udpSocket){
        ThrowErrnoException(env,"java/io/IOException",errno);
    }
    return udpSocket;
}
static ssize_t SendDataGramToSocket(JNIEnv*env,jobject obj,int sd,const struct sockaddr_in *address,const char *buffer,size_t bufferSize){
    // Send data buffer to the socket
    LogAddress(env, obj, "Sending to", address);
    ssize_t sentSize = sendto(sd, buffer, bufferSize, 0,
                              (const sockaddr*) address,
                              sizeof(struct sockaddr_in));

    // If send is failed
    if (-1 == sentSize)
    {
        // Throw an exception with error number
        ThrowErrnoException(env, "java/io/IOException", errno);
    }
    else if (sentSize > 0)
    {
        LogMessage(env, obj, "Sent %d bytes: %s", sentSize, buffer);
    }

    return sentSize;
}
static ssize_t ReceiveDatagramFromSocket(JNIEnv*env,
                                         jobject obj,
                                         int sd,
                                         struct sockaddr_in *address,
                                         char* buffer,size_t bufferSize){
    socklen_t addressLength=sizeof(struct sockaddr_in);
    LogMessage(env,obj,"Receiving from the socket...");
    ssize_t recvSize=recvfrom(sd,buffer,bufferSize,0,(struct sockaddr*)address,&addressLength);
    if(-1==recvSize){
        ThrowErrnoException(env,"java/io/IOException",errno);
    }
    else{
        LogMessage(env,obj,"Received from",address);
        buffer[recvSize]=NULL;
        if(recvSize>0){
            LogMessage(env,obj,"Received %d bytes:%s",recvSize,buffer);
        }
    }
    return recvSize;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_minhuizhu_echo_EchoClientActivity_nativeStartUdpClient(JNIEnv *env, jobject instance,
                                                                jstring ip, jint port,
                                                                jstring message) {
   int clientSocket= NewUdpSocket(env,instance);
    if(NULL==env->ExceptionOccurred()){
        struct sockaddr_in address;
        memset(&address,0, sizeof(address));
        address.sin_family=PF_INET;
        const char * ipAddress=env->GetStringUTFChars(ip,NULL);
        if(NULL==ipAddress)
            goto exit;
        int result=inet_aton(ipAddress,&(address.sin_addr));
        env->ReleaseStringUTFChars(ip,ipAddress);
        if(0==result){
            ThrowErrnoException(env,"java/io/IOException",errno);
            goto exit;
        }
        address.sin_port=htons(port);
        const char*messageText=env->GetStringUTFChars(message,NULL);
        if(NULL==messageText)
            goto exit;
        jsize messageSize=env->GetStringUTFLength(message);
        SendDataGramToSocket(env,instance,clientSocket,&address,messageText,messageSize);
        env->ReleaseStringUTFChars(message,messageText);
        if(NULL!=env->ExceptionOccurred())
            goto exit;
        char buffer[MAX_BUFFER_SIZE];
        memset(&address,0, sizeof(address));
        ReceiveDatagramFromSocket(env,instance,clientSocket,&address,buffer,MAX_BUFFER_SIZE);
    }
    exit:
    if (clientSocket > 0)
    {
        close(clientSocket);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_minhuizhu_echo_LocalEchoActivity_nativeStartLocalServer(JNIEnv *env, jobject instance,
                                                                 jstring name_) {
    const char *name = env->GetStringUTFChars(name_, 0);



    env->ReleaseStringUTFChars(name_, name);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_minhuizhu_echo_EchoServerActivity_nativeStartTcpServer(JNIEnv *env, jobject instance,
                                                                jint port) {
 int serverSocket=   NewTcpSocket(env,instance);
    if(NULL==env->ExceptionOccurred()){
        BindSocketToPort(env,instance,serverSocket,(unsigned short )port);
        if(NULL!=env->ExceptionOccurred()){
            goto exit;
        }
        int clientSocket=AcceptOnSocket(env,instance,serverSocket);
        if(NULL!=env->ExceptionOccurred())
            goto exit;
        char buffer[MAX_BUFFER_SIZE];
        ssize_t recvSize;
        ssize_t sentSize;
        while(1){
            recvSize=ReceiveFromSocket(env,instance,clientSocket,buffer,MAX_BUFFER_SIZE);
            if((0==recvSize)||(NULL!=env->ExceptionOccurred()))
                break;
            sentSize=SendToSocket(env,instance,clientSocket,buffer,(size_t)recvSize);
            if((0==sentSize)||(NULL!=env->ExceptionOccurred()))
                break;
        }
        close(clientSocket);
    }

exit:
    if(serverSocket>0){
        close(serverSocket);
    }
}



extern "C"
JNIEXPORT void JNICALL
Java_com_minhuizhu_echo_EchoServerActivity_nativeStartUdpServer(JNIEnv *env, jobject instance,
                                                                jint port) {


}


