#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/wait.h>
#define MKDIR_STATUS 1
#define MKNOD_STATUS 2
#define RMDIR_STATUS 3
#define REMOVE_STATUS 4
#define READDR_STATUS 5
#define RENAME_STATUS 6
#define TRUNCATE_STATUS 7
#define WRITE_STATUS 8
#define READ_STATUS 9
#define OPEN_STATUS 10

static  const  char * dirpath = "/home/yulia/Documents";
char key[100] = "9(ku@AW1[Lmvgax6q`5Y2Ry?+sF!^HKQiBXCUSe&0M.b%rI'7d)o4~VfZ*{#:}ETt$3J-zpc]lnh8,GwP_ND|jO";
char encv1[10] = "encv1_";
char encv2[10] = "encv2_";

static int lastCommand = 0;

void writeWarning(char * str){
	FILE * logFile = fopen("/home/yulia/fs.log", "a");
	time_t t;
	time ( &t );
	struct tm * timeinfo = localtime (&t);
	fprintf(logFile, "WARNING::%02d%02d%02d-%02d:%02d:%02d::%s\n", timeinfo->tm_year-100, timeinfo->tm_mon+1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, str);
	fclose(logFile);
}

void writeInfo(char * str){
	FILE * logFile = fopen("/home/yulia/fs.log", "a");
	time_t t;
	time ( &t );
	struct tm * timeinfo = localtime (&t);
	fprintf(logFile, "INFO::%02d%02d%02d-%02d:%02d:%02d::%s\n", timeinfo->tm_year-100, timeinfo->tm_mon+1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, str);
	fclose(logFile);
}

void encription1WithLength(char* enc, int length) {
	if(strcmp(enc, ".") == 0 || strcmp(enc, "..") == 0)return;
	for(int i = length; i >= 0; i--){
		if(enc[i]=='/')break;
		if(enc[i]=='.'){
			length = i;
			break;
		}
	}
	int start = 0;
	for(int i = 0; i < length; i++){
		if(enc[i] == '/'){
			start = i+1;
			break;
		}
	}
    for ( int i = start; i < length; i++) {
		if(enc[i]=='/')continue;
        for (int j = 0; j < 87; j++) {
            if(enc[i] == key[j]) {
                enc[i] = key[(j+10) % 87];
                break;
            }
        }
    }
}

void decription1WithLength(char * enc, int length){
	if(strcmp(enc, ".") == 0 || strcmp(enc, "..") == 0)return;
	if(strstr(enc, "/") == NULL)return;
	for(int i = length; i >= 0; i--){
		if(enc[i]=='/')break;
		if(enc[i]=='.'){
			length = i;
			break;
		}
	}
	int start = length;
	for(int i = 0; i < length; i++){
		if(enc[i] == '/'){
			start = i+1;
			break;
		}
	}
    for ( int i = start; i < length; i++) {
		if(enc[i]=='/')continue;
        for (int j = 0; j < 87; j++) {
            if(enc[i] == key[j]) {
                enc[i] = key[(j+77) % 87];
                break;
            }
        }
    }
}

void encription2(char * path){
	FILE * file = fopen(path, "rb");
	int count = 0;
	char topath[1000];
	sprintf(topath, "%s.%03d", path, count);
	void * buffer = malloc(1024);
	while(1){
		size_t readSize = fread(buffer, 1, 1024, file);
		if(readSize == 0)break;
		FILE * op = fopen(topath, "w");
		fwrite(buffer, 1, readSize, op);
		fclose(op);
		count++;
		sprintf(topath, "%s.%03d", path, count);
	}
	free(buffer);
	fclose(file);
	remove(path);
}

void decription2(char * path){
	FILE * check = fopen(path, "r");
	if(check != NULL)return;
	FILE * file = fopen(path, "w");
	int count = 0;
	char topath[1000];
	sprintf(topath, "%s.%03d", path, count);
	void * buffer = malloc(1024);
	while(1){
		FILE * op = fopen(topath, "rb");
		if(op == NULL)break;
		size_t readSize = fread(buffer, 1, 1024, op);
		fwrite(buffer, 1, readSize, file);
		fclose(op);
		remove(topath);
		count++;
		sprintf(topath, "%s.%03d", path, count);
	}
	free(buffer);
	fclose(file);
}

void encrypt2Directory(char * dir){
	DIR *dp;
	struct dirent *de;
	dp = opendir(dir);
	if (dp == NULL)
		return;
	char dirPath[1000];
	char filePath[1000];
	while ((de = readdir(dp)) != NULL) {
		if(strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)continue;
		if(de->d_type == DT_DIR){
			sprintf(dirPath, "%s/%s", dir, de->d_name);
			encrypt2Directory(dirPath);
		}else if(de->d_type == DT_REG){
			sprintf(filePath, "%s/%s", dir, de->d_name);
			encription2(filePath);
		}
	}
	closedir(dp);
}

void decrypt2Directory(char * dir){
	DIR *dp;
	struct dirent *de;
	dp = opendir(dir);
	if (dp == NULL)
		return;
	char dirPath[1000];
	char filePath[1000];
	while ((de = readdir(dp)) != NULL) {
		if(strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)continue;
		if(de->d_type == DT_DIR){
			sprintf(dirPath, "%s/%s", dir, de->d_name);
			decrypt2Directory(dirPath);
		}else if(de->d_type == DT_REG){
			sprintf(filePath, "%s/%s", dir, de->d_name);
			filePath[strlen(filePath)-4] = '\0';
			decription2(filePath);
		}
	}
	closedir(dp);
}

void encription1(char* enc) {
	encription1WithLength(enc, strlen(enc));
}

void decription1(char* enc){
	decription1WithLength(enc, strlen(enc));
}

static  int  xmp_getattr(const char *path, struct stat *stbuf){
	char * enc1Ptr = strstr(path, encv1);
	if(lastCommand == MKNOD_STATUS || lastCommand == MKDIR_STATUS){

	}else{
		if(enc1Ptr != NULL)
			decription1(enc1Ptr);
	}
	printf("DEBUG getattr %d %s\n", lastCommand, path);
	char * enc2Ptr = strstr(path, encv2);
	int res;
	char fpath[1000];
	sprintf(fpath,"%s%s", dirpath, path);
	printf("%s\n", fpath);
	res = lstat(fpath, stbuf);
	if (res == -1){
		if(enc2Ptr == NULL){
			return -errno;
		}else{
			if(strstr(enc2Ptr, "/") == NULL){
				return -errno;
			}else{
				sprintf(fpath,"%s%s.000", dirpath, path);
				lstat(fpath, stbuf);
				int count = 0;
				struct stat st;
				int sizeCount = 0;
				while(1){
					if(stat(fpath, &st) < 0){
						break;
					}
					count++;
					sprintf(fpath, "%s%s.%03d", dirpath, path, count);
					sizeCount += st.st_size;
				}
				stbuf->st_size = sizeCount;
				return 0;
			}
		}
	}
	printf("%d\n", res);
	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){

	printf("DEBUGGING %s\n", path);
	char * enc1Ptr = strstr(path, encv1);
	if(enc1Ptr != NULL)
		decription1(enc1Ptr);
	char * enc2Ptr = strstr(path, encv2);

	printf("\n\nDEBUG readdir\n\n");

	char fpath[1000];
	if(strcmp(path,"/") == 0){
		path=dirpath;
		sprintf(fpath,"%s",path);
	} else sprintf(fpath, "%s%s", dirpath, path);

	int res = 0;
	DIR *dp;
	struct dirent *de;
	(void) offset;
	(void) fi;
	dp = opendir(fpath);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if(enc2Ptr != NULL){
			if(de->d_type == DT_REG ){
				if(strcmp(de->d_name+(strlen(de->d_name)-4), ".000") == 0){
					de->d_name[strlen(de->d_name)-4] = '\0';
					res = (filler(buf, de->d_name, &st, 0));
				}
			}else{
				res = (filler(buf, de->d_name, &st, 0));
			}
		}else{
			if(enc1Ptr != NULL)
				encription1(de->d_name);
			res = (filler(buf, de->d_name, &st, 0));
		}
		if(res!=0) break;
	}
	closedir(dp);
	lastCommand = READDR_STATUS;
	return 0;
}

static int xmp_mkdir(const char *path, mode_t mode){

	lastCommand = MKDIR_STATUS;

	printf("\n\nDEBUG mkdir %s\n\n", path);

	char * enc1Ptr = strstr(path, encv1);
	if(enc1Ptr != NULL){
		int length = strlen(enc1Ptr);
		for(int i = length; i >= 0; i--){
			if(enc1Ptr[i] == '/'){
				length = i;
				break;
			}
		}
		decription1WithLength(enc1Ptr, length);
	}

	char fpath[1000];
	sprintf(fpath, "%s%s",dirpath,path);
	printf("%s\n", fpath);
	int res;

	res = mkdir(fpath, mode);
	char str[100];
	sprintf(str, "MKDIR::%s", path);
	writeInfo(str);
	if (res == -1)
		return -errno;
	return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev){

	lastCommand = MKNOD_STATUS;

	char * enc1Ptr = strstr(path, encv1);
	if(enc1Ptr != NULL){
		int length = strlen(enc1Ptr);
		printf("%d\n", length);
		for(int i = length; i >= 0; i--){
			if(enc1Ptr[i] == '/'){
				length = i;
				break;
			}
		}
		printf("%d\n", length);
		decription1WithLength(enc1Ptr, length);
	}
	printf("\n\nDEBUG mknod %s\n\n", path);

	char fpath[1000];
	if(strcmp(path,"/") == 0){
		path=dirpath;
		sprintf(fpath,"%s",path);
	} else sprintf(fpath, "%s%s",dirpath,path);
	int res;

	if (S_ISREG(mode)) {
		res = open(fpath, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(fpath, mode);
	else
		res = mknod(fpath, mode, rdev);
	char str[100];
	sprintf(str, "CREAT::%s", path);
	writeInfo(str);

	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_unlink(const char *path) {

	char * enc1Ptr = strstr(path, encv1);
	if(enc1Ptr != NULL){
		decription1(enc1Ptr);
	}

	printf("\n\nDEBUG unlink\n\n");

	char fpath[1000];
	if(strcmp(path,"/") == 0){
		path=dirpath;
		sprintf(fpath,"%s",path);
	} else sprintf(fpath, "%s%s",dirpath,path);
	int res;

	char str[100];
	sprintf(str, "REMOVE::%s", path);
	writeWarning(str);

	res = unlink(fpath);
	if (res == -1)
		return -errno;
	lastCommand = REMOVE_STATUS;
	return 0;
}

static int xmp_rmdir(const char *path) {
	lastCommand = RMDIR_STATUS;

	char * enc1Ptr = strstr(path, encv1);
	if(enc1Ptr != NULL)
		decription1(enc1Ptr);

	printf("\n\nDEBUG rmdir\n\n");

	char fpath[1000];
	sprintf(fpath, "%s%s",dirpath,path);
	int res;

	res = rmdir(fpath);
	char str[100];
	sprintf(str, "RMDIR::%s", path);
	writeWarning(str);
	if (res == -1)
		return -errno;
	return 0;
}

static int xmp_rename(const char * from, const char * to) {
	lastCommand = RENAME_STATUS;

	char * encrFrom = strstr(from, encv2);
	char * encrTo = strstr(to, encv2);

	printf("\n\nDEBUG rename\n\n");

	char ffrom[1000];
	char fto[1000];
	sprintf(ffrom, "%s%s",dirpath, from);
	sprintf(fto, "%s%s",dirpath, to);
	int res;

	int dirIndex = 0;
	int length = strlen(fto);
	for(int i = length; i >= 0; i--){
		if(fto[i] == '/'){
			dirIndex = i;
			break;
		}
	}

	char dir[1000];
	strncpy(dir, fto, dirIndex);
	pid_t id = fork();
	if(id){
		wait(NULL);
	}else{
		char * arg[]={"mkdir", "-p", dir, NULL};
		execv("/bin/mkdir", arg);
	}

	res = rename(ffrom, fto);
	char str[100];
	sprintf(str, "RENAME::%s::%s", from, to);
	writeInfo(str);
	if (res == -1)
		return -errno;
	else{
		if(encrFrom == NULL && encrTo != NULL){
			encrypt2Directory(fto);
		} else if(encrFrom != NULL && encrTo == NULL){
			decrypt2Directory(fto);
		}
	}
	return 0;
}

static int xmp_truncate(const char *path, off_t size) {
	lastCommand = TRUNCATE_STATUS;

	printf("\n\nDEBUG truncate\n\n");

	char * enc1Ptr = strstr(path, encv1);
	if(enc1Ptr != NULL)
		decription1(enc1Ptr);

	char fpath[1000];
	sprintf(fpath, "%s%s",dirpath,path);
	int res;

	res = truncate(fpath, size);
	if (res == -1)
		return -errno;
	return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi){

	char * enc1Ptr = strstr(path, encv1);
	char * enc2Ptr = strstr(path, encv2);
	if(lastCommand == MKNOD_STATUS){
		if(enc1Ptr != NULL){
			int length = strlen(enc1Ptr);
			for(int i = length; i >= 0; i--){
				if(enc1Ptr[i] == '/'){
					length = i;
					break;
				}
			}
			decription1WithLength(enc1Ptr, length);
		}
	}else{
		if(enc1Ptr != NULL)
			decription1(enc1Ptr);
	}
	printf("\n\nDEBUG open %d %s\n\n", lastCommand, path);

	char fpath[1000];

	if(enc2Ptr != NULL){
		sprintf(fpath, "%s%s.000",dirpath,path);
	}else{
		sprintf(fpath, "%s%s",dirpath,path);
	}

	printf("%s\n", fpath);
	int res;

	res = open(fpath, fi->flags);
	if (res == -1)
		return -errno;
	close(res);
	return 0;
}

static int xmp_read(const char * path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
	
	char * enc1Ptr = strstr(path, encv1);
	if(lastCommand == MKNOD_STATUS){
		if(enc1Ptr != NULL){
			int length = strlen(enc1Ptr);
			for(int i = length; i >= 0; i--){
				if(enc1Ptr[i] == '/'){
					length = i;
					break;
				}
			}
			decription1WithLength(enc1Ptr, length);
		}
	}else{
		if(enc1Ptr != NULL)
			decription1(enc1Ptr);
	}
	lastCommand = READ_STATUS;
	printf("\n\nDEBUG read %s\n\n", path);

	char fpath[1000];
	int fd;
	int res = 0;

	(void) fi;
	sprintf(fpath, "%s%s",dirpath,path);
	
	fd = open(fpath, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;
	close(fd);
	return res;
}

static int xmp_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	
	char * enc1Ptr = strstr(path, encv1);
	if(lastCommand == MKNOD_STATUS){
		if(enc1Ptr != NULL){
			int length = strlen(enc1Ptr);
			for(int i = length; i >= 0; i--){
				if(enc1Ptr[i] == '/'){
					length = i;
					break;
				}
			}
			decription1WithLength(enc1Ptr, length);
		}
	}else{
		if(enc1Ptr != NULL)
			decription1(enc1Ptr);
	}

	lastCommand = WRITE_STATUS;
	printf("\n\nDEBUG write %s\n\n", path);
	
	char fpath[1000];
	sprintf(fpath, "%s%s", dirpath, path);
	int fd;
	int res;

	(void) fi;
	fd = open(fpath, O_WRONLY);
	
	if (fd == -1)
		return -errno;

	char str[100];
	sprintf(str, "WRITE::%s", path);
	writeInfo(str);

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;
	close(fd);
	return res;
}


static struct fuse_operations xmp_oper = {
	.getattr = xmp_getattr,
	.readdir = xmp_readdir,
	.read = xmp_read,
	.mkdir = xmp_mkdir,
	.mknod = xmp_mknod,
	.unlink = xmp_unlink,
	.rmdir = xmp_rmdir,
	.rename = xmp_rename,
	.truncate = xmp_truncate,
	.open = xmp_open,
	.read = xmp_read,
	.write = xmp_write,

};

int  main(int  argc, char *argv[]){
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}


/*

HOW TO COMPILE

cd Documents/M4/Shift
gcc -Wall `pkg-config fuse --cflags` ssfs.c -o ssfs `pkg-config fuse --libs`
mkdir fuse
./ssfs fuse

no 1.Buka terminal lagi
cd Documents
mv M1 encv1_M1
mv encv1_M1 M1



no 4
cd fuse

mkdir oke
touch siap.c
rmdir oke
unlink siap.c

INFO mencatat : mkdir, rename, write, touch
WARNING mencatat : rmdir, unlink
lihat di home/yulia ada file fs.log 

cd ..
sudo umount ssfs

*/