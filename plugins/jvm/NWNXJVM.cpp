/***************************************************************************
    NWNXJava.cpp - Implementation of the CNWNXJVM class.
    (c) 2009 Bernhard Stoeckner (elven@swordcoast.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ***************************************************************************/

#include <string.h>
#include <string>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>

#include "NWNXJVM.h"
#include "FunctionHooks.h"
#include "org_nwnx_nwnx2_jvm_registerNWScript.cpp"
#include "org_nwnx_nwnx2_jvm_registerNWScript_addon.cpp"

jclass jclassNWObject;
jmethodID jmethodNWObjectCreate;
jmethodID jmethodNWObjectgetOid;

jclass jclassNWVector;
jmethodID jmethodNWVectorCreate;
jmethodID jmethodNWVectorgetX;
jmethodID jmethodNWVectorgetY;
jmethodID jmethodNWVectorgetZ;

jclass jclassNWLocation;
jmethodID jmethodNWLocationCreate;
jmethodID jmethodNWLocationgetArea;
jmethodID jmethodNWLocationgetX;
jmethodID jmethodNWLocationgetY;
jmethodID jmethodNWLocationgetZ;
jmethodID jmethodNWLocationgetFacing;

jclass jclassNWEffect;
jmethodID jmethodNWEffectCreate;
jmethodID jmethodNWEffectgetOid;
jclass jclassNWItemProperty;
jmethodID jmethodNWItemPropertyCreate;
jmethodID jmethodNWItemPropertygetOid;

jclass jclassNWNotInContextException;
jclass jclassNWInvalidObjectException;
jclass jclassNWInvalidEffectException;
jclass jclassNWInvalidItemPropertyException;

jclass jclassSCORCOListener;
jmethodID jmethodSCO;
jmethodID jmethodRCO;

jclass jclassResmanListener;
jmethodID jmethodResmanDemandRes;


jclass jclassConv;
jmethodID jmethodConvToNative;
jmethodID jmethodConvFromNative;

unsigned long nwn_contextDepth;
extern unsigned long j_callCounter;
extern void j_resetPerEventMappings(JNIEnv*);

bool j_debugNWScriptCalls = true;
bool j_printEvents = true;

extern PLUGINLINK *pluginLink;

#define JE(jnienvtok,code) if (jnienvtok->ExceptionCheck()) { \
jnienvtok->ExceptionDescribe();\
jnienvtok->FatalError("while executing native code " #code);\
}

#define J(jnienvtok,code) jnienvtok->code; JE(jnienvtok,code);


JavaVM *jvm_global;

int WriteSCO(WPARAM, LPARAM);
int ReadSCO(WPARAM, LPARAM);
int DemandResource(WPARAM, LPARAM);

CNWNXJVM::CNWNXJVM() {
	confKey = (char*) "JVM";
	bHooked = 1;
}

int systemStartup(WPARAM l, LPARAM i) {
	HANDLE handleSCO = HookEvent("NWServer/RCO", ReadSCO);
	HANDLE handleRCO = HookEvent("NWServer/SCO", WriteSCO);
	HANDLE handleResMan = HookEvent("NWNX/ResMan/DemandResource", DemandResource);
	if (!handleSCO || !handleRCO || !handleResMan)
		printf("Cannot hook SCORCO or ResMan events!\n");
	return 0;
}

CNWNXJVM::~CNWNXJVM() {
	if (this->jvm->AttachCurrentThread((void**)&(this->env), NULL) < 0)
		fprintf(stderr, "Attaching native NWN thread to JVM failed, cannot call shutdown()! Damn!");
	else {
		J(this->env, CallStaticVoidMethod(this->cls_initListener, this->jmethodJavaShutdown));

		if (this->jvm->DetachCurrentThread() < 0)
			fprintf(stderr, "Detaching our thread from JVM failed.");
	}
}

bool CNWNXJVM::OnCreate(gline *config, const char *LogDir) {
	char log[128];
	struct stat statbuf;
	int i = 0;
	sprintf (log, "%s/nwnx_jvm.txt", LogDir);

	if (!CNWNXBase::OnCreate(config,log))
		return false;

	Log(0, "NWNXJVM\n");

	HookEvent("NWNX/Core/PluginsLoaded", systemStartup);

	if(!pluginLink) {
		Log (0, "Plugin link not accessible\n");
		return false;
	} else {
		Log (0, "Plugin link: %08lX\n", pluginLink);
	}


	if (HookFunctions(0 != strcmp("", (*nwnxConfig)[confKey]["scorcoListener"].c_str()))) {
		bHooked=1;
		Log(0,"* Module loaded successfully.\n");
	} else {
		bHooked=0;
		Log(0,"* Signature recognition failed.\n");
		return false;
	}

    return LaunchVM();
}

bool CNWNXJVM::LaunchVM() {
	JavaVMInitArgs vm_args;

	Log(0, "Fetching & setting default VM Initialization args...\n");
	vm_args.version = JNI_VERSION_1_4;
	int numOptions = 2;

	j_printEvents = atoi((*nwnxConfig)[confKey]["printEvents"].c_str()) != 0;
	j_debugNWScriptCalls = atoi((*nwnxConfig)[confKey]["debugNWScriptCalls"].c_str()) != 0;

	const char* vmOptions = (*nwnxConfig)[confKey]["vmOptions"].c_str();
	char* vmOpt = strdup(vmOptions);
	vmOpt = strtok(vmOpt, " ");
	while (vmOpt != NULL) {
		numOptions++;
		vmOpt = strtok(NULL, " ");
	}

	JavaVMOption* options = new JavaVMOption[numOptions];
	numOptions = 2;

	const char* classPath = (*nwnxConfig)[confKey]["classpath"].c_str();
	Log(0, "Constructing classpath...\n");
	char cp [20 + strlen(classPath)];
	sprintf(cp, "-Djava.class.path=%s", classPath);
	Log(0, cp); Log(0, "\n");
	options[0].optionString = cp;

	// Don't hook into SIGINT, TERM, QUIT
	options[1].optionString = (char*) "-Xrs";

	vmOpt = strdup(vmOptions);
	vmOpt = strtok(vmOpt, " ");
	while (vmOpt != NULL) {
		printf("Setting JVM option: %s\n", vmOpt);
		options[numOptions++].optionString = vmOpt;
		vmOpt = strtok(NULL, " ");
	}

	vm_args.nOptions = numOptions;
	vm_args.options = options;

	Log(0, "Creating the Virtual Machine...\n");
	if (JNI_CreateJavaVM(&(this->jvm),(void**)&(this->env), &vm_args) != 0)
		JE(env, "Cannot create Java VM!");

	jvm_global = this->jvm;

	if (this->jvm->AttachCurrentThread((void**)&(this->env), NULL) < 0)
		this->env->FatalError("Attaching native NWN thread to JVM failed.");

	const char* initListener = (*nwnxConfig)[confKey]["initListener"].c_str();
	Log(0, "Looking for initListener: "); Log(0, initListener); Log(0, "...\n");
	this->cls_initListener = J(this->env, FindClass(initListener));
	this->cls_initListener = (jclass) J(this->env, NewGlobalRef(this->cls_initListener));

	Log(0, "Looking up setup method...\n");
	this->jmethodJavaSetup    = J(this->env, GetStaticMethodID(this->cls_initListener, "setup", "()V"));
	Log(0, "Looking up init method...\n");
	this->jmethodJavaInit     = J(this->env, GetStaticMethodID(this->cls_initListener, "init", "()V"));
	Log(0, "Looking up shutdown method...\n");
	this->jmethodJavaShutdown = J(this->env, GetStaticMethodID(this->cls_initListener, "shutdown", "()V"));

	Log(0, "Calling setup method...\n");
	J(this->env, CallStaticVoidMethod(this->cls_initListener, this->jmethodJavaSetup));

	Log(0, "Looking for class org/nwnx/nwnx2/jvm/NWScript...\n");
	this->cls_nwscript = J(this->env, FindClass("org/nwnx/nwnx2/jvm/NWScript"));

	Log(0, "Registering custom functions...\n");
	jint ret = this->registerNWScriptNatives();
	JE(this->env, this->registerNWScriptNatives());
	if (ret != 0)
		this->env->FatalError("RegisterNative failed!");

	Log(0, "Registering addon functions...\n");
	ret = this->registerNWScriptAddonNatives();
	JE(this->env, this->registerNWScriptAddonNatives());
	if (ret != 0)
		this->env->FatalError("RegisterNative failed!");

	const char* scorcoListener = (*nwnxConfig)[confKey]["scorcoListener"].c_str();
	Log(0, "Looking for SCORCOListener: "); Log(0, scorcoListener); Log(0, "...\n");
	jclassSCORCOListener = J(this->env, FindClass(scorcoListener));

	Log(0, "Looking up SCO method...\n");
	jmethodSCO	 = J(this->env, GetStaticMethodID(jclassSCORCOListener, "sco", "(Ljava/lang/String;[B)V"));
	Log(0, "Looking up RCO method...\n");
	jmethodRCO	 = J(this->env, GetStaticMethodID(jclassSCORCOListener, "rco", "(Ljava/lang/String;)[B"));

	const char* resmanListener = (*nwnxConfig)[confKey]["resmanListener"].c_str();
	Log(0, "Looking for resmanListener: "); Log(0, resmanListener); Log(0, "...\n");
	jclassResmanListener = J(this->env, FindClass(resmanListener));

	Log(0, "Looking up demandRes method...\n");
	jmethodResmanDemandRes        = J(this->env, GetStaticMethodID(jclassResmanListener, "demandRes", "(Ljava/lang/String;)[B"));


	Log(0, "Looking for class org/nwnx/nwnx2/jvm/NWObject...\n");
	jclassNWObject = J(this->env, FindClass("org/nwnx/nwnx2/jvm/NWObject"));
	jclassNWObject = (jclass) J(this->env, NewGlobalRef(jclassNWObject));
	jmethodNWObjectCreate = J(this->env, GetStaticMethodID(jclassNWObject, "apply", "(I)Lorg/nwnx/nwnx2/jvm/NWObject;"));
	jmethodNWObjectgetOid = J(this->env, GetMethodID(jclassNWObject, "oid", "()I"));

	Log(0, "Looking for class org/nwnx/nwnx2/jvm/NWVector...\n");
	jclassNWVector = J(this->env, FindClass("org/nwnx/nwnx2/jvm/NWVector"));
	jclassNWVector = (jclass) J(this->env, NewGlobalRef(jclassNWVector));
	jmethodNWVectorCreate = J(this->env, GetStaticMethodID(jclassNWVector, "apply", "(FFF)Lorg/nwnx/nwnx2/jvm/NWVector;"));
	jmethodNWVectorgetX = J(this->env, GetMethodID(jclassNWVector, "x", "()F"));
	jmethodNWVectorgetY = J(this->env, GetMethodID(jclassNWVector, "y", "()F"));
	jmethodNWVectorgetZ = J(this->env, GetMethodID(jclassNWVector, "z", "()F"));

	Log(0, "Looking for class org/nwnx/nwnx2/jvm/NWLocation...\n");
	jclassNWLocation = J(this->env, FindClass("org/nwnx/nwnx2/jvm/NWLocation"));
	jclassNWLocation = (jclass) J(this->env, NewGlobalRef(jclassNWLocation));
	jmethodNWLocationCreate = J(this->env, GetStaticMethodID(jclassNWLocation, "apply", "(Lorg/nwnx/nwnx2/jvm/NWObject;FFFF)Lorg/nwnx/nwnx2/jvm/NWLocation;"));
	jmethodNWLocationgetArea = J(this->env, GetMethodID(jclassNWLocation, "area", "()Lorg/nwnx/nwnx2/jvm/NWObject;"));
	jmethodNWLocationgetX = J(this->env, GetMethodID(jclassNWLocation, "x", "()F"));
	jmethodNWLocationgetY = J(this->env, GetMethodID(jclassNWLocation, "y", "()F"));
	jmethodNWLocationgetZ = J(this->env, GetMethodID(jclassNWLocation, "z", "()F"));
	jmethodNWLocationgetFacing = J(this->env, GetMethodID(jclassNWLocation, "facing", "()F"));

	Log(0, "Looking for class org/nwnx/nwnx2/jvm/NWEffect...\n");
	jclassNWEffect = J(this->env, FindClass("org/nwnx/nwnx2/jvm/NWEffect"));
	jclassNWEffect = (jclass) J(this->env, NewGlobalRef(jclassNWEffect));
	jmethodNWEffectCreate = J(this->env, GetStaticMethodID(jclassNWEffect, "apply", "(I)Lorg/nwnx/nwnx2/jvm/NWEffect;"));
	jmethodNWEffectgetOid = J(this->env, GetMethodID(jclassNWEffect, "oid", "()I"));

	Log(0, "Looking for class org/nwnx/nwnx2/jvm/NWItemProperty...\n");
	jclassNWItemProperty = J(this->env, FindClass("org/nwnx/nwnx2/jvm/NWItemProperty"));
	jclassNWItemProperty = (jclass) J(this->env, NewGlobalRef(jclassNWItemProperty));
	jmethodNWItemPropertyCreate = J(this->env, GetStaticMethodID(jclassNWItemProperty, "apply", "(I)Lorg/nwnx/nwnx2/jvm/NWItemProperty;"));
	jmethodNWItemPropertygetOid = J(this->env, GetMethodID(jclassNWItemProperty, "oid", "()I"));

	Log(0, "Looking for class org/nwnx/nwnx2/jvm/NWvNotInContextException...\n");
	jclassNWNotInContextException = J(this->env, FindClass("org/nwnx/nwnx2/jvm/NWNotInContextException"));
	jclassNWNotInContextException = (jclass) J(this->env, NewGlobalRef(jclassNWNotInContextException));
	Log(0, "Looking for class org/nwnx/nwnx2/jvm/NWInvalidObjectException...\n");
	jclassNWInvalidObjectException = J(this->env, FindClass("org/nwnx/nwnx2/jvm/NWInvalidObjectException"));
	jclassNWInvalidObjectException = (jclass) J(this->env, NewGlobalRef(jclassNWInvalidObjectException));
	Log(0, "Looking for class org/nwnx/nwnx2/jvm/NWInvalidEffectException...\n");
	jclassNWInvalidEffectException = J(this->env, FindClass("org/nwnx/nwnx2/jvm/NWInvalidEffectException"));
	jclassNWInvalidEffectException = (jclass) J(this->env, NewGlobalRef(jclassNWInvalidEffectException));
	Log(0, "Looking for class org/nwnx/nwnx2/jvm/NWInvalidItemPropertyException...\n");
	jclassNWInvalidItemPropertyException = J(this->env, FindClass("org/nwnx/nwnx2/jvm/NWInvalidItemPropertyException"));
	jclassNWInvalidItemPropertyException = (jclass) J(this->env, NewGlobalRef(jclassNWInvalidItemPropertyException));

	const char* eventListener = (*nwnxConfig)[confKey]["eventListener"].c_str();
	Log(0, "Looking for eventListener: "); Log(0, eventListener); Log(0, "...\n");
	this->cls_eventListener = J(this->env, FindClass(eventListener));
	this->cls_eventListener = (jclass) J(this->env, NewGlobalRef(this->cls_eventListener));

	Log(0, "Looking up event method...\n");
	this->jmethodJavaEvent = J(this->env, GetStaticMethodID(this->cls_eventListener, "event", "(Lorg/nwnx/nwnx2/jvm/NWObject;Ljava/lang/String;)V"));

	const char* tokenListener = (*nwnxConfig)[confKey]["tokenListener"].c_str();
	Log(0, "Looking for tokenListener: "); Log(0, tokenListener); Log(0, "...\n");
	this->cls_tokenListener = J(this->env, FindClass(tokenListener));
	this->cls_tokenListener = (jclass) J(this->env, NewGlobalRef(this->cls_tokenListener));

	Log(0, "Looking up token method...\n");
	this->jmethodJavaToken = J(this->env, GetStaticMethodID(this->cls_tokenListener, "token", "(Lorg/nwnx/nwnx2/jvm/NWObject;Ljava/lang/String;)V"));

	const char* convClass = (*nwnxConfig)[confKey]["convClass"].c_str();
	Log(0, "Looking for convClass: "); Log(0, convClass); Log(0, "...\n");
	jclassConv = J(this->env, FindClass(convClass));
	jclassConv = (jclass) J(this->env, NewGlobalRef(jclassConv));
	jmethodConvToNative = J(this->env, GetStaticMethodID(jclassConv, "toNative", "(Ljava/lang/String;)[B"));
	jmethodConvFromNative = J(this->env, GetStaticMethodID(jclassConv, "fromNative", "([B)Ljava/lang/String;"));

	Log(0, "Calling init method...\n");
	J(this->env, CallStaticVoidMethod(this->cls_initListener, this->jmethodJavaInit));

	if (this->jvm->DetachCurrentThread() < 0)
		this->env->FatalError("Detaching native NWN thread from JVM failed.");

	Log(0, "Java VM Created and running!\n");
	return true;
}

char* CNWNXJVM::OnRequest (char *gameObject, char* request, char* parameters) {
	timeval t1;
	if (j_printEvents)
		gettimeofday(&t1, NULL);

	Log(2, "(S) Request: \"%s\"\n", request);
	Log(2, "(S) Params:  \"%s\"\n", parameters);

	if (this->jvm->AttachCurrentThread((void**)&(this->env), NULL) < 0)
		this->env->FatalError("Attaching native NWN thread to JVM failed.");

	const char* oid_c = strtok(parameters, " ");
	const char* event  = strtok(NULL, " ");

	if (NULL == event) {
		printf("Usage error, need format: ObjectToString(OBJECT_SELF) event_name\n");
		return NULL;
	}

	nwn_contextDepth += 1;

	jlong oid = strtol(oid_c, 0, 16);
	jobject obj_self = J(this->env, CallStaticObjectMethod(jclassNWObject, jmethodNWObjectCreate, oid));
	jstring event_s = J(this->env, NewStringUTF(event));

	unsigned long previousCallCounter = j_callCounter;

	if (NULL != obj_self) {
		if (strcmp(request, "EVENT") == 0)
			J(this->env, CallStaticVoidMethod(this->cls_eventListener, this->jmethodJavaEvent, obj_self, event_s));
		if (strcmp(request, "TOKEN") == 0)
			J(this->env, CallStaticVoidMethod(this->cls_tokenListener, this->jmethodJavaToken, obj_self, event_s));
	}

	nwn_contextDepth -= 1;

	unsigned long calls = j_callCounter - previousCallCounter;
	j_callCounter = previousCallCounter;

	if (nwn_contextDepth == 0 && this->jvm->DetachCurrentThread() < 0)
		this->env->FatalError("Detaching native NWN thread from JVM failed.");

	if (nwn_contextDepth == 0)
		j_resetPerEventMappings(env);

	if (NULL == obj_self) {
		printf("%c          %-20s NWObject.apply(%8llx) == NULL!\n", request[0], event, oid);

	} else if (j_printEvents) {
		timeval t2; gettimeofday(&t2, NULL);
		long usec = (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);
		long se = usec / 1000000;
		long ms = usec % 1000000;

		printf("%c %8llx %-20s %6ld s %8ld u %12ld calls (d: %ld)\n", request[0], oid, event, se, ms, calls, nwn_contextDepth);
	}

	return NULL;
}

int WriteSCO(WPARAM p, LPARAM a) {
	SCORCOStruct* s = (SCORCOStruct*) p;

	JNIEnv *env;
	jvm_global->AttachCurrentThread((void **)&env, NULL);
	jstring key_s = J(env, NewStringUTF(s->key));
	jbyteArray arr = J(env, NewByteArray(s->size));
	J(env, SetByteArrayRegion(arr, 0, s->size, (jbyte*) s->pData));
	J(env, CallStaticVoidMethod(jclassSCORCOListener, jmethodSCO, key_s, arr));

	jvm_global->DetachCurrentThread();
	return 0;
};

int ReadSCO(WPARAM p, LPARAM a) {
	SCORCOStruct* s = (SCORCOStruct*) p;
	
	JNIEnv *env;
	jvm_global->AttachCurrentThread((void **)&env, NULL);
	
	jstring key_s = J(env, NewStringUTF(s->key));
	jbyteArray ret = (jbyteArray) J(env, CallStaticObjectMethod(jclassSCORCOListener, jmethodRCO, key_s));
	if (ret == NULL)
		return 0;

	unsigned char* data = (unsigned char*) J(env, GetByteArrayElements(ret, NULL));
	int sz = J(env, GetArrayLength(ret));
	jvm_global->DetachCurrentThread();
	s->pData = data;
	s->size = sz;
	return 1;
};

int DemandResource(WPARAM p, LPARAM a) {
	ResManDemandResStruct* s = (ResManDemandResStruct*) p;

	JNIEnv *env;
	jvm_global->AttachCurrentThread((void **)&env, NULL);

	jstring resref = J(env, NewStringUTF(s->resref));
	jbyteArray ret = (jbyteArray) J(env, CallStaticObjectMethod(jclassResmanListener, jmethodResmanDemandRes, resref));
	if (ret == NULL)
		return 0;

	unsigned char* data = (unsigned char*) J(env, GetByteArrayElements(ret, NULL));
	int sz = J(env, GetArrayLength(ret));
	jvm_global->DetachCurrentThread();
	s->pData = data;
	s->size = sz;
	return 1;
};
