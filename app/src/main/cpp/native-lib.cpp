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
static void AcceptOnSocket(JNIEnv*env,jobject obj,int sd){
   struct sockaddr_in address;
    socklen_t addressLength= sizeof(address);

}
extern "C"
JNIEXPORT void JNICALL
Java_com_minhuizhu_echo_EchoClientActivity_nativeStartUdpClient(JNIEnv *env, jobject instance,
                                                                jstring ip_, jint port,
                                                                jstring message_) {
    const char *ip = env->GetStringUTFChars(ip_, 0);
    const char *message = env->GetStringUTFChars(message_, 0);


    env->ReleaseStringUTFChars(ip_, ip);
    env->ReleaseStringUTFChars(message_, message);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_minhuizhu_echo_LocalEchoActivity_nativeStartLocalServer(JNIEnv *env, jobject instance,
                                                                 jstring name_) {
    const char *name = env->GetStringUTFChars(name_, 0);

    // TODO

    env->ReleaseStringUTFChars(name_, name);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_minhuizhu_echo_EchoServerActivity_nativeStartTcpServer(JNIEnv *env, jobject instance,
                                                                jint port) {

    // TODO

}
extern "C"
JNIEXPORT void JNICALL
Java_com_minhuizhu_echo_EchoServerActivity_nativeStartUdpServer(JNIEnv *env, jobject instance,
                                                                jint port) {

    // TODO

}

extern "C"
jstring
Java_com_minhuizhu_echo_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
