/*
  Big Brother File System
  Copyright (C) 2012 Joseph J. Pfeiffer, Jr., Ph.D. <pfeiffer@cs.nmsu.edu>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.

  This code is derived from function prototypes found /usr/include/fuse/fuse.h
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  His code is licensed under the LGPLv2.
  A copy of that code is included in the file fuse.h

  The point of this FUSE filesystem is to provide an introduction to
  FUSE.  It was my first FUSE filesystem as I got to know the
  software; hopefully, the comments in this code will help people who
  follow later to get a gentler introduction.

  This might be called a no-op filesystem:  it doesn't impose
  filesystem semantics on top of any other existing structure.  It
  simply reports the requests that come in, and passes them to an
  underlying filesystem.  The information is saved in a logfile named
  bbfs.log, in the directory from which you run bbfs.
*/

/******************  OUR CODE (COMPRESSED FS)**********************
 * 
 * We only use structs to keep information.
 * 
 * We have a struct for metadata, files, blocks and a structure 
 * that acts as the storage that keeps all the different blocks(all_blocks).
 * We have a similar struct tat acts as a storage for all the 
 * metadata of the files(root_files).
 * 
 * When we copy a file that has a same block as the other in the mounted directory,
 * that block is not saved in the storage.
 * 
 *Can read and write files in size of multiples of 4KB but implemented our bb_read and 
 * bb_write to read and write from an offset and for different sizes.
 * 
 * To see the contents of a file, use command cat.
 * 
 */
////
#include "config.h"
#include "params.h"
//library for the hash function
#include <openssl/sha.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif

#include "log.h"
#define BLOCK_SIZE 4096
#define PATH_SIZE 300
#define MAX_BLOCKS 16
#define MAX_FILES 10

typedef struct bb_block{
    unsigned char buffer[BLOCK_SIZE + 1];//keeps all the data
    unsigned char hash[SHA_DIGEST_LENGTH];
    //number of blocks, needed only for all_blocks array
    int num_blocks;
}bb_block;

typedef struct bb_metadata{
    int num_children;//if file is directory, this show how many files it has in it
    int size;//SIZE OF FILE
    char name[30];
    int num_block;//number of blocks
    char fullpath[PATH_SIZE];
    mode_t mode;
}bb_metadata;

typedef struct bb_file {
    int block[MAX_BLOCKS];//periexei indexes twn blocks ths apo ton pinaka all_blocks
    struct bb_file *files;
    bb_metadata *metadata;
}bb_file;

typedef struct bb_all_blocks{
    bb_block *blocks;
    int num_blocks;
}bb_all_blocks;

//the mount directory's files
bb_file *root_files;
bb_all_blocks *all_blocks;

//  All the paths I see are relative to the root of the mounted
//  filesystem.  In order to get to the underlying filesystem, I need to
//  have the mountpoint.  I'll save it away early on in main(), and then
//  whenever I need a path for something I'll call this to construct
//  it.
static void bb_fullpath(char fpath[PATH_MAX], const char *path)
{
    strcpy(fpath, BB_DATA->rootdir);
    strncat(fpath, path, PATH_MAX); // ridiculously long paths will
				    // break here

    log_msg("    bb_fullpath:  rootdir = \"%s\", path = \"%s\", fpath = \"%s\"\n",
	    BB_DATA->rootdir, path, fpath);
}

bb_file * check_bbfile(const char *path){
    // check if file exists in the  fullpath (under mountdir) and return it
    char fpath[PATH_MAX];
    const char delim[15] = "/";
    char token[30], *temp;
    int i = 0, flag =0;
    bb_file *cur = root_files;
    
    log_msg("\ncheck_bb_file\n");
    //strcpy(fpath,path);
    if(strcmp(path, "/") == 0){
        log_msg("\nexiting check, it was root\n");
        return cur;
    }
    
    temp = strtok(path, delim);
    strcpy(token, temp);
    /* walk through other tokens */
    while( temp != NULL ) {
        flag =0;
        log_msg("\nin while, CHILDREN: %d, token: %s\n", cur->metadata->num_children, token);
        for(i = 0;i < cur->metadata->num_children; i++){
            log_msg("\nin for token: %s, name:%s\n", token,  cur->files[i].metadata->name);
            if(strcmp(token, cur->files[i].metadata->name) == 0){
                cur = &cur->files[i];
                flag = 1;
                log_msg("\nin if, children: %d\n",cur->metadata->num_children);
                break;
            }
        }
        log_msg("\nafter for\n");
        //searched all kids and found nothing
        if(flag == 0){
            log_msg("\nFLAG = 0\n");
            break;
        }
        log_msg("\nBefore strtok\n");
        temp = strtok(NULL, delim);
        log_msg("\nAfter strtok\n");
        if(temp != NULL)
            strcpy(token, temp);
        log_msg("\nafter strcpy\n");
    }
    //an vrhke full path match
    if(flag){
        log_msg("\nexiting check\n");
        return cur;
    }
        
    log_msg("\nreturning NULL\n");
    return NULL;

}

///////////////////////////////////////////////////////////
//
// Prototypes for all these functions, and the C-style comments,
// come from /usr/include/fuse.h
//
/** Get file attributes.
 *
 * Similar to stat().  The 'st_dev' and 'st_blksize' fields are
 * ignored.  The 'st_ino' field is ignored except if the 'use_ino'
 * mount option is given.
 */

//// CHANGE SIZE
int bb_getattr(const char *path, struct stat *statbuf)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg("\nbb_getattr(path=\"%s\", statbuf=0x%08x)\n",
	  path, statbuf);
    bb_fullpath(fpath, path);
    bb_file *file = check_bbfile(path);
    if(file == NULL){
        log_msg("\nfile was ENOENT\n");
        return -ENOENT;
    }
    //statbuf->st_ino = file->number;
    statbuf->st_blocks = file->metadata->num_block/8;
    statbuf->st_blocks = BLOCK_SIZE;
    statbuf->st_mode = file->metadata->mode;
    statbuf->st_size = file->metadata->size;

    log_msg("\nbb_getattr IS OK\n");
    return retstat;
}

/** Read the target of a symbolic link
 *
 * The buffer should be filled with a null terminated string.  The
 * buffer size argument includes the space for the terminating
 * null character.  If the linkname is too long to fit in the
 * buffer, it should be truncated.  The return value should be 0
 * for success.
 */
// Note the system readlink() will truncate and lose the terminating
// null.  So, the size passed to to the system readlink() must be one
// less than the size passed to bb_readlink()
// bb_readlink() code by Bernardo F Costa (thanks!)
int bb_readlink(const char *path, char *link, size_t size)
{
    int retstat;
    char fpath[PATH_MAX];

    log_msg("\nbb_readlink(path=\"%s\", link=\"%s\", size=%d)\n",
	  path, link, size);
    bb_fullpath(fpath, path);

    retstat = 0;//log_syscall("readlink", readlink(fpath, link, size - 1), 0);
    if (retstat >= 0) {
	link[retstat] = '\0';
	retstat = 0;
	log_msg("    link=\"%s\"\n", link);
    }

    return retstat;
}
char *get_name(char *path){
    const char delim[15] = "/";
    char *token, old_token[PATH_MAX];
    /* get the first token */
    token = strtok(path, delim);

    log_msg("\nget_name\n");
    /* walk through other tokens */
    while( token != NULL ) {
        strcpy(old_token, token);
        token = strtok(NULL, delim);
    }
    strcpy(path, old_token);
    return path;

}

void add_on_root_dir(bb_file *newfile,  char *path){
    bb_file *cur = root_files;

   const char delim[15] = "/";
   char *token;
   
   log_msg("\nadd on root dir\n");
   /* get the first token */
   token = strtok(path, delim);
   
   /* walk through other tokens */
   while( token != NULL ) {
      for(int j = 0; j < cur->metadata->num_children; j++){
        if(strcmp(cur->files[j].metadata->name,token) == 0){
            cur = &cur->files[j];
            break;
            // sub-dir found!
        }
      }
      token = strtok(NULL, delim);
   }

   (cur->metadata->num_children)++;

   cur->files = realloc(cur->files, cur->metadata->num_children * sizeof(bb_file));
   if(cur->files == NULL)
       log_error("\nadd on root dir realloc\n");
   cur->metadata->size += newfile->metadata->size;
   cur->files[cur->metadata->num_children -1] = *newfile;
      log_msg("\nLeve add dir\n");
}



/** Create a directory */
int bb_mkdir(const char *path, mode_t mode)
{
    int retstat = 0;
    char fpath[PATH_MAX], ppath[PATH_MAX], pppath[PATH_MAX];
    log_msg("\nbb_mkdir(path=\"%s\", mode=0%3o) PATH_MAX  %d\n",
	    path, mode, PATH_MAX);
    bb_fullpath(fpath, path);
    strcpy(ppath, path);
    strcpy(pppath, path);
    
    bb_file *newfile = (bb_file *)malloc(sizeof(bb_file));
    //newfile->block = NULL;
    newfile->files = (bb_file *)malloc(sizeof(bb_file));
    //num???
    newfile->metadata = (bb_metadata *)malloc(sizeof(bb_metadata));
    newfile->metadata->num_children = 0;
    newfile->metadata->size = 0;
    strcpy(newfile->metadata->fullpath,fpath);
    newfile->metadata->num_block = 0;
    newfile->metadata->mode  = S_IRWXO | S_IRWXG | S_IRWXU | S_IFDIR;

    char name[30];
    strcpy(name, get_name(ppath));
    strcpy(newfile->metadata->name, name);
    log_msg("MKDIR 1");
//     for(int i = 0; i < MAX_BLOCKS; i++){
//         *(newfile->block[i].buffer) = '\0';
//         strcpy(newfile->block[i].hash, "00000000000000000000");
//     }

    log_msg("MKDIR OK");
    add_on_root_dir(newfile, pppath);
    return retstat;
}


int add_on_root(bb_file *newfile,  char *path){
    bb_file *cur = root_files;
/*
    (cur->metadata->num_children)++;
    cur->files[0] = &newfile;*/

   const char delim[15] = "/";
   char *token;
   int flag = 0;
   /* get the first token */
   token = strtok(path, delim);

   log_msg("\n=============================================\n");
   /* walk through other tokens */
   while( token != NULL ) {
        log_msg("\nCUR NUM nChildren %d\n", cur->metadata->num_children);
        log_msg("\nCUR size %d\n", cur->metadata->size);
        log_msg("\nCUR name %s\n", cur->metadata->name);
      for(int j = 0; j < cur->metadata->num_children; j++){
        if(strcmp(cur->files[j].metadata->name,token) == 0){
            cur = &cur->files[j];
            // sub-dir found!
            flag = 1;
            break;
        }
      }
      
      //get nex token, if token NULL, and flag =0, then
      //we reached the end and cur points to parent directory
      //where new file will be added
      token = strtok(NULL, delim);
      if(flag == 0 && token ==NULL){
          break;
      }
      else if(flag == 0){
        //if we are still not on last token but there is no match,
        //then file is in a nonexistent subdirectory, return ENOENT
        
        //bb_mkdir(path, S_IRWXO | S_IRWXG | S_IRWXU | S_IFDIR);
        //bb_file *dir = check_bbfile(path);
        //cur = dir;
        log_msg("\nSub dir does not exist!\n");  
        return ENOENT;
      }
      flag = 0;
      
   }

   log_msg("\nEXITED WHILE\n");
   (cur->metadata->num_children)++;
   cur->metadata->size += newfile->metadata->size;
   log_msg("\nCUR NUM nChildren %d\n", cur->metadata->num_children);
   log_msg("\nCUR size %d\n", cur->metadata->size);
   log_msg("\nCUR name %s\n", cur->metadata->name);
   
   log_msg("\nbefore realloc\n");
   cur->files = realloc(cur->files, cur->metadata->num_children * sizeof(bb_file));
   if(cur->files == NULL){
       log_msg("\nERROR realloc\n");
       log_error("add on root realloc");
   }
   cur->files[cur->metadata->num_children -1] = *newfile;
   log_msg("\nChildren of root: %d\n", cur->metadata->num_children);
   return 0;
}


/** Create a file node
 *
 * There is no create() operation, mknod() will be called for
 * creation of all non-directory, non-symlink nodes.
 */
// shouldn't that comment be "if" there is no.... ?
int bb_mknod(const char *path, mode_t mode, dev_t dev)
{
    int retstat = 0;
    char fpath[PATH_MAX], ppath[PATH_MAX];

    log_msg("\nbb_mknod(path=\"%s\", mode=0%3o, dev=%lld)\n",
	  path, mode, dev);
    bb_fullpath(fpath, path);
    strcpy(ppath, path);
    bb_file *newfile = (bb_file *)malloc(sizeof(bb_file));
    for(int i = 0; i < MAX_BLOCKS; i++)
        newfile->block[i] = -1;
    
    newfile->files = NULL;
    newfile->metadata = (bb_metadata *)malloc(sizeof(bb_metadata));
    newfile->metadata->num_children = 0;
    newfile->metadata->size = 0;
    strcpy(newfile->metadata->fullpath,fpath);
    newfile->metadata->num_block = 0;
    newfile->metadata->mode  = S_IRWXO | S_IRWXG | S_IRWXU | S_IFREG;

    char name[30];
    strcpy(name, get_name(ppath));
    strcpy(newfile->metadata->name, name);
    log_msg("\nBefore strcpy\n");
    //for(int i = 0; i < MAX_BLOCKS; i++){
    //    strcpy(newfile->block[0].buffer,"\0");
    //    strcpy(newfile->block[0].hash, "00000000000000000000");
    //}

     add_on_root(newfile, path);
     log_msg("\nMKNOD - SUCCESS\n");

    return retstat;
}


void remove_from_root(const char *path){
    // check if file exists in the  fullpath (under mountdir) and return it
    bb_file *cur = root_files;
    bb_file *parent = cur;
    char *token;
    char delim [15]= "/";
    int i, j, flag = 0;
    token = strtok(path, delim);

    log_msg("\nremove from root\n");
    /* walk through other tokens */
    while( token != NULL ) {
        for(i = 0; i < cur->metadata->num_children; i++){
            if(strcmp(cur->files[i].metadata->name,token) == 0){
                log_msg("\nNAME: %s\n", cur->files[i].metadata->name);
                parent = cur;
                cur = &cur->files[i];
                log_msg("\nNAME CHILD: %s\n", cur->metadata->name);
                // sub-dir found!
                flag = 1;
                break;
            }
        }
    
        token = strtok(NULL, delim);
        if(flag == 1 && token == NULL){
          log_msg("\nDONE: %s\n", cur->metadata->name);
          break;
        }
        else if(flag == 0){
    
            log_msg("\nSub dir does not exist!\n");  
            return ENOENT;
        }
        flag = 0;
    }
    log_msg("\nAfter while\n");

    parent->metadata->size -= parent->files[i].metadata->size;
    parent->metadata->num_block -= parent->files[i].metadata->num_block;

    parent->metadata->num_children = parent->metadata->num_children - 1;
    log_msg("\nBefore for kids:%d, i: %d\n", parent->metadata->num_children, i);
     for(j = i; j < parent->metadata->num_children; j++){
          log_msg("\n hellooooo\n");
         log_msg("\n child:%d, name: %s\n",j,parent->files[j].metadata->name);
        parent->files[j] = parent->files[j+1];
    }
    
    
    log_msg("\n after for\n");

    log_msg("\nAfter for kids:%d name: %s\n", parent->metadata->num_children, parent->metadata->name);

    //if(parent->metadata->num_children > 0)
    //free(&parent->files[parent->metadata->num_children]); 
    log_msg("\nparent size: %d, bb_file size %d\n", sizeof(parent->files), sizeof(bb_file));
    parent->files = realloc(parent->files, sizeof(bb_file)*parent->metadata->num_children);
    log_msg("\nparent size: %d\n", sizeof(parent->files));
    if(parent->files == NULL)
        log_error("\nremove from root realloc\n");

    for(int k = 0; k < parent->metadata->num_children; k++){
        log_msg("\nwhy\n");
        log_msg("\n%d: %s\n",k, parent->files[k].metadata->name);
    }
    log_msg("\nExited remove from root\n");
    //free(&parent->files[j]);
}

/** Remove a file */
int bb_unlink(const char *path)
{
    char fpath[PATH_MAX];

    log_msg("\nbb_unlink(path=\"%s\")\n",
	    path);
    bb_fullpath(fpath, path);

    remove_from_root(path);
    log_msg("\nExiting unlink\n");
    return 0;//log_syscall("unlink", unlink(fpath), 0);
}

/** Remove a directory */
int bb_rmdir(const char *path)
{
    char fpath[PATH_MAX];

    log_msg("\nbb_rmdir(path=\"%s\")\n",
	    path);
    bb_fullpath(fpath, path);
    remove_from_root(path);
    return 0;//log_syscall("rmdir", rmdir(fpath), 0);
}

/** Create a symbolic link */
// The parameters here are a little bit confusing, but do correspond
// to the symlink() system call.  The 'path' is where the link points,
// while the 'link' is the link itself.  So we need to leave the path
// unaltered, but insert the link into the mounted directory.
int bb_symlink(const char *path, const char *link)
{
    char flink[PATH_MAX];

    log_msg("\nbb_symlink(path=\"%s\", link=\"%s\")\n",
	    path, link);
    bb_fullpath(flink, link);

    return 0;//log_syscall("symlink", symlink(path, flink), 0);
}

/** Rename a file */
// both path and newpath are fs-relative
int bb_rename(const char *path, const char *newpath)
{
    char fpath[PATH_MAX];
    char fnewpath[PATH_MAX];

    log_msg("\nbb_rename(fpath=\"%s\", newpath=\"%s\")\n",
	    path, newpath);
    bb_fullpath(fpath, path);
    bb_fullpath(fnewpath, newpath);

    return 0;//log_syscall("rename", rename(fpath, fnewpath), 0);
}

/** Create a hard link to a file */
int bb_link(const char *path, const char *newpath)
{
    char fpath[PATH_MAX], fnewpath[PATH_MAX];

    log_msg("\nbb_link(path=\"%s\", newpath=\"%s\")\n",
	    path, newpath);
    bb_fullpath(fpath, path);
    bb_fullpath(fnewpath, newpath);

    return 0;//log_syscall("link", link(fpath, fnewpath), 0);
}

/** Change the permission bits of a file */
int bb_chmod(const char *path, mode_t mode)
{
    char fpath[PATH_MAX];

    log_msg("\nbb_chmod(fpath=\"%s\", mode=0%03o)\n",
	    path, mode);
    bb_fullpath(fpath, path);

    return 0;//log_syscall("chmod", chmod(fpath, mode), 0);
}

/** Change the owner and group of a file */
int bb_chown(const char *path, uid_t uid, gid_t gid)

{
    char fpath[PATH_MAX];

    log_msg("\nbb_chown(path=\"%s\", uid=%d, gid=%d)\n",
	    path, uid, gid);
    bb_fullpath(fpath, path);

    return 0;//log_syscall("chown", chown(fpath, uid, gid), 0);
}

/** Change the size of a file */
int bb_truncate(const char *path, off_t newsize)
{
    char fpath[PATH_MAX];

    log_msg("\nbb_truncate(path=\"%s\", newsize=%lld)\n",
	    path, newsize);
    bb_fullpath(fpath, path);
    /// our code
    bb_file *newfile = check_bbfile(path);
    newfile->metadata->size = newsize;
    return newsize;//log_syscall("truncate", truncate(fpath, newsize), 0);
}

/** Change the access and/or modification times of a file */
/* note -- I'll want to change this as soon as 2.6 is in debian testing */
int bb_utime(const char *path, struct utimbuf *ubuf)
{
    char fpath[PATH_MAX];

    log_msg("\nbb_utime(path=\"%s\", ubuf=0x%08x)\n",
	    path, ubuf);
    bb_fullpath(fpath, path);

    //return log_syscall("utime", utime(fpath, ubuf), 0);
    return 0;
}

/** File open operation
 *
 * No creation, or truncation flags (O_CREAT, O_EXCL, O_TRUNC)
 * will be passed to open().  Open should check if the operation
 * is permitted for the given flags.  Optionally open may also
 * return an arbitrary filehandle in the fuse_file_info structure,
 * which will be passed to all file operations.
 *
 * Changed in version 2.2
 */
int bb_open(const char *path, struct fuse_file_info *fi)
{
     int retstat = 0;
    char fpath[PATH_MAX];

    log_msg("\nbb_open(path=\"%s\", fi=0x%08x)\n",
	  path, fi);

    bb_fullpath(fpath, path);

    bb_file *dir = check_bbfile(path);
    log_msg("\nafter check\n");

    if(dir == NULL){
        log_msg("\nenoent\n");
        return -ENOENT;
    }
    if(S_ISDIR(dir->metadata->mode)){
        log_msg("\nait is a dir\n");
        return -1;
    }
    log_msg("\nbefore fi\n");
    fi->fh = (intptr_t) dir;
    log_msg("\nEXiting open!\n");
    return retstat;
}

void file_init(bb_file *file, struct stat sb, char *path){

    log_msg("\nfile_init\n");
    file->metadata->size = sb.st_size;
    strcpy(file->metadata->name, path);
    file->metadata->num_block = sb.st_blocks/8;

}

/** Read data from an open file
 *
 * Read should return exactly the number of bytes requested except
 * on EOF or error, otherwise the rest of the data will be
 * substituted with zeroes.  An exception to this is when the
 * 'direct_io' mount option is specified, in which case the return
 * value of the read system call will reflect the return value of
 * this operation.
 *
 * Changed in version 2.2
 */
// I don't fully understand the documentation above -- it doesn't
// match the documentation for the read() system call which says it
// can return with anything up to the amount of data requested. nor
// with the fusexmp code which returns the amount of data also
// returned by read.
int bb_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    log_msg("\nRead (path=\"%s\"), size %d\n", path, size);
    int j;
    int which_block, blocks;
    bb_file * newfile;

    struct stat sb;

    int ret = bb_getattr(path, &sb);
    if(S_ISDIR(sb.st_mode) || ret == -ENOENT){
        return -1;
    }

    //file_init(&newfile, sb);
    log_msg("\n1\n");
    blocks = size/BLOCK_SIZE ;
    log_msg("\n2 blocks %d\n", blocks);
    which_block = offset/BLOCK_SIZE;
    log_msg("\n3 which block %d\n", which_block);
    
    newfile = check_bbfile(path);
    log_msg("\n4 offset %d\n", offset);

    //write to buf from blocks
    char temp[BLOCK_SIZE + 1];
    temp[BLOCK_SIZE] = '\0';
    // first block, read from offset
    int begin_point = offset%BLOCK_SIZE;
    //buf = (char *)malloc(sizeof(char)*BLOCK_SIZE*blocks);
    int index = newfile->block[which_block];
    log_msg("\n5 begin_point %d\n", begin_point);
    log_msg("\n6 index for all_blocks %d\n", index);
    if(newfile->metadata->size > 0 && which_block < newfile->metadata->num_block ){
        log_msg("\hash %s\n", all_blocks->blocks[index].hash);
        log_msg("\nbuffer %s\n", all_blocks->blocks[index].buffer);
        log_msg("\nbuffer %s\n", &(all_blocks->blocks[index].buffer[begin_point]));
        strncpy(temp,&(all_blocks->blocks[index].buffer[begin_point]), BLOCK_SIZE - begin_point);
        log_msg("\n6 %s\n", all_blocks->blocks[index].buffer);
    }
    strcpy(buf,temp);
    log_msg("\n7 %s\n", buf);
    // intermediate blocks, read them fully
    for(j = which_block+1; j  < blocks + which_block -1; j++){
        log_msg("\nintermediate block\n");
        strcpy(temp,all_blocks->blocks[newfile->block[j]].buffer);
        strncat(buf,temp,BLOCK_SIZE);
        log_msg("\n%s\n",temp);
    }

    //last block
    if(blocks > 1){
        log_msg("\nlast block\n");
        if((begin_point + size%BLOCK_SIZE) == 0){//perfect fit
            strcpy(temp,all_blocks->blocks[newfile->block[j]].buffer);
            strncat(buf,temp,BLOCK_SIZE);
        }
        else{
            strncpy(temp, all_blocks->blocks[newfile->block[j]].buffer,begin_point + size%BLOCK_SIZE);
            log_msg("\n%s\n",temp);
            strncat(buf,temp,begin_point + size%BLOCK_SIZE);
        }
    }
    log_msg("\nbb_read(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
	    path, buf, size, offset, fi);
    // no need to get fpath on this one, since I work from fi->fh not the path
    //log_fi(fi);
    log_msg("\nFINAL BUF %s\n", buf);
    return size;//log_syscall("pread", pread(fi->fh, buf, size, offset), 0);
}

/** Write data to an open file
 *
 * Write should return exactly the number of bytes requested
 * except on error.  An exception to this is when the 'direct_io'
 * mount option is specified (see read operation).
 *
 * Changed in version 2.2
 */
// As  with read(), the documentation above is inconsistent with the
// documentation for the write() system call.
int bb_write(const char *path, const char *buf, size_t size, off_t offset,
	     struct fuse_file_info *fi)
{
    int j, flag =0;
    struct stat sb;
    int begin_point = 0;
    int ssize = BLOCK_SIZE;
    unsigned const char temp_buf[BLOCK_SIZE];
    unsigned char temp_hash[SHA_DIGEST_LENGTH];
    
    log_msg("\nWrite %s\n, buf: %s, size: %d", path, buf, size);
    int ret = bb_getattr(path, &sb);
    
    log_msg("\n 1 \n");
    if( S_ISDIR(sb.st_mode) || ret == -ENOENT){
        log_msg("\n 2 \n");
        return -1;
    }
    log_msg("\n 3 \n");
    int new_blocks = size/BLOCK_SIZE;
        log_msg("\n 4 new blocks %d \n",new_blocks);
        
    //if buffer is not a multiple of 4KB
    //and >4KB, then it need an extra block for that extra
    //data to write.
    if(size%BLOCK_SIZE > 0 && size>BLOCK_SIZE){
        new_blocks += 1;
        //size_to_write = BLOCK_SIZE;
    }
    if(size < BLOCK_SIZE){
        new_blocks += 1;//giati to size/BLOCK_SIZE = 0  
        ssize = size;
    }
    int which_block = offset/BLOCK_SIZE;
    log_msg("\n 5 which block %d \n",which_block);
    bb_file *newfile = check_bbfile(path);
    if(S_ISDIR(newfile->metadata->mode)){
        log_msg("\nERROR WRITE: File is directory\n");
        return -1;
    }
    //    newfile->block = (bb_block *)malloc(sizeof(bb_block)*MAX_BLOCKS);
    log_msg("\noffset:%d buf[offset]:%s \n", offset, buf + offset%BLOCK_SIZE);
    log_msg("\n 7 \n");
    if(newfile->metadata->size > 0){
        log_msg("\nfile has data 8 \n");
        begin_point = which_block;
    }
    
    //first block
    if(size > 0){
        log_msg("\nfirst block, size to write: %d\n",ssize - offset%BLOCK_SIZE);
        strcpy(temp_buf, buf + offset%BLOCK_SIZE);
        log_msg("\nbefore actual copy %s\n", temp_buf);

        strncpy(temp_buf, buf + offset%BLOCK_SIZE, ssize - offset%BLOCK_SIZE);
        log_msg("\ninitial %d buf %s\n", ssize, temp_buf);
         newfile->metadata->size += strlen(buf);
        newfile->metadata->num_block = newfile->metadata->num_block + 1;

        flag = 0;
        //create hash for new block
        SHA1(temp_buf, BLOCK_SIZE, temp_hash);

        log_msg("\nSHA %s %d \n", temp_hash, strlen(temp_hash));
        
        //check in the table of all the blocks if there is a similar hash
        for(j = 0; j < all_blocks->num_blocks; j++){
            log_msg("\nhi\n");
            log_msg("\nhash %d : %s\n", j, all_blocks->blocks[j].hash);
            //an h thesh sto pinaka einai kenh, den exei idio block ston pinaka
            //ara break kai valto
           if(strcmp((const char *)all_blocks->blocks[j].hash,(const char *)"00000000000000000000")==0){
               log_msg("\nNO BLOCKS MATCH\n");
                break;
           }
            if(strncmp((const char *)(temp_hash), (const char *)all_blocks->blocks[j].hash, SHA_DIGEST_LENGTH) == 0){
                log_msg("\nSAME HASH %s 11 \n", all_blocks->blocks[j].hash);
                newfile->block[newfile->metadata->num_block -1] = j;//now file points to the j-th block of the storage
                log_msg("\nj: %d, newfile->block[j]: %d \n", j, newfile->block[newfile->metadata->num_block -1]);
                flag = 1;
                break;
            }
 
        }
        //new block
        if(flag == 0){
            newfile->block[newfile->metadata->num_block -1] = j;
            all_blocks->num_blocks = all_blocks->num_blocks + 1;
            log_msg("\nALL BLOCKS: %d\n",all_blocks->num_blocks);
            all_blocks->blocks = realloc(all_blocks->blocks,sizeof(bb_block)*(all_blocks->num_blocks));
            if(all_blocks->blocks == NULL)
                log_error("\nwrite realloc\n");
            strncpy(all_blocks->blocks[j].buffer, temp_buf, size);
            log_msg("\n12 %s \n", all_blocks->blocks[j].buffer);
            strncpy(all_blocks->blocks[j].hash, temp_hash, SHA_DIGEST_LENGTH);
            log_msg("\n13 %s \n", all_blocks->blocks[j].hash);
            //all_blocks->blocks[j] = newfile->block[i];
        }
    }
    
    //intermediate blocks (all BLOCK_SIZE)
    for(int i = begin_point +1; i < new_blocks + begin_point -1; i++){
        unsigned char temp_hash2[SHA_DIGEST_LENGTH];
        strncpy(temp_buf,buf, BLOCK_SIZE);
        log_msg("\n%s 9 \n", temp_buf);
        newfile->metadata->size += strlen(buf);
        newfile->metadata->num_block = newfile->metadata->num_block + 1;
        //deixnei an den yparxei auto to block
        flag = 0;
        //create hash for new block
        SHA1(temp_buf, BLOCK_SIZE, temp_hash2);

        log_msg("\nSHA %s %d \n", temp_hash2, strlen(temp_hash2));
        
        //check in the table of all the blocks if there is a similar hash
        for(j = 0; j < all_blocks->num_blocks; j++){
            log_msg("\nhi\n");
            log_msg("\nhash %d : %s\n", j, all_blocks->blocks[j].hash);
            //an h thesh sto pinaka einai kenh, den exei idio block ston pinaka
            //ara break kai valto
           if(strcmp((const char *)all_blocks->blocks[j].hash,(const char *)"00000000000000000000")==0){
               log_msg("\nNO BLOCKS MATCH\n");
                break;
           }
            if(strncmp((const char *)(temp_hash2), (const char *)all_blocks->blocks[j].hash, SHA_DIGEST_LENGTH) == 0){
                log_msg("\nSAME HASH %s 11 \n", all_blocks->blocks[j].hash);
                newfile->block[newfile->metadata->num_block -1] = j;//now file points to the j-th block of the storage
                log_msg("\nj: %d, newfile->block[j]: %d \n", j, newfile->block[newfile->metadata->num_block -1]);
                flag = 1;
                break;
            }
 
        }
        //new block
        if(flag == 0){
            newfile->block[newfile->metadata->num_block -1] = j;
            all_blocks->num_blocks = all_blocks->num_blocks + 1;
            log_msg("\nALL BLOCKS: %d\n",all_blocks->num_blocks);
            all_blocks->blocks = realloc(all_blocks->blocks,sizeof(bb_block)*(all_blocks->num_blocks));
            if(all_blocks->blocks == NULL)
                log_error("\nwrite realloc\n");
            strncpy(all_blocks->blocks[j].buffer, temp_buf, size);
            log_msg("\n12 %s \n", all_blocks->blocks[j].buffer);
            strncpy(all_blocks->blocks[j].hash, temp_hash2, SHA_DIGEST_LENGTH);
            log_msg("\n13 %s \n", all_blocks->blocks[j].hash);
            //all_blocks->blocks[j] = newfile->block[i];
        }
    }

    //last block
    if(new_blocks > 1){
        log_msg("\nlast block\n");
        if((offset%BLOCK_SIZE + size%BLOCK_SIZE) == 0){//perfect fit
            strncpy(temp_buf,buf, BLOCK_SIZE);
        }
        else{
            strncpy(temp_buf, buf,offset%BLOCK_SIZE + size%BLOCK_SIZE);
            log_msg("\n%s\n",temp_buf);
        }
        unsigned char temp_hash1[SHA_DIGEST_LENGTH];
        log_msg("\n%s 9 \n", temp_buf);
        newfile->metadata->size += strlen(buf);
        newfile->metadata->num_block = newfile->metadata->num_block + 1;
        //deixnei an den yparxei auto to block
        flag = 0;
        //create hash for new block
        SHA1(temp_buf, BLOCK_SIZE, temp_hash1);
        //strncpy(temp_hash,temp_hash_ptr, SHA_DIGEST_LENGTH);
        log_msg("\nSHA %s %d \n", temp_hash1, strlen(temp_hash1));
        
        //check in the table of all the blocks if there is a similar hash
        for(j = 0; j < all_blocks->num_blocks; j++){
            log_msg("\nhi\n");
            log_msg("\nhash %d : %s\n", j, all_blocks->blocks[j].hash);
            //an h thesh sto pinaka einai kenh, den exei idio block ston pinaka
            //ara break kai valto
           if(strcmp((const char *)all_blocks->blocks[j].hash,(const char *)"00000000000000000000")==0){
               log_msg("\nNO BLOCKS MATCH\n");
                break;
           }
            if(strncmp((const char *)(temp_hash1), (const char *)all_blocks->blocks[j].hash, SHA_DIGEST_LENGTH) == 0){
                log_msg("\nSAME HASH %s 11 \n", all_blocks->blocks[j].hash);
                newfile->block[newfile->metadata->num_block -1] = j;//now file points to the j-th block of the storage
                log_msg("\nj: %d, newfile->block[j]: %d \n", j, newfile->block[newfile->metadata->num_block -1]);
                flag = 1;
                break;
            }
 
        }
        //new block
        if(flag == 0){
            newfile->block[newfile->metadata->num_block -1] = j;
            all_blocks->num_blocks = all_blocks->num_blocks + 1;
            log_msg("\nALL BLOCKS: %d\n",all_blocks->num_blocks);
            all_blocks->blocks = realloc(all_blocks->blocks,sizeof(bb_block)*(all_blocks->num_blocks));
            if(all_blocks->blocks == NULL)
                log_error("\nwrite realloc\n");
            strncpy(all_blocks->blocks[j].buffer, temp_buf, size);
            log_msg("\n12 %s \n", all_blocks->blocks[j].buffer);
            strncpy(all_blocks->blocks[j].hash, temp_hash1, SHA_DIGEST_LENGTH);
            log_msg("\n13 %s \n", all_blocks->blocks[j].hash);
            //all_blocks->blocks[j] = newfile->block[i];
        }
    }
    log_msg("\nbb_write(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
	    path, buf, size, offset, fi
	    );
    log_msg("\nALL BLOCKS: %d\n",all_blocks->num_blocks);
    log_msg("\n12 %s \n", all_blocks->blocks[j].buffer);
    log_msg("\n13 %s \n", all_blocks->blocks[j].hash);
    log_msg("\nnewfile->block[j]: %d \n",newfile->block[newfile->metadata->num_block -1]);
    // no need to get fpath on this one, since I work from fi->fh not the path
    log_fi(fi);

    return size;
}

/** Get file system statistics
 *
 * The 'f_frsize', 'f_favail', 'f_fsid' and 'f_flag' fields are ignored
 *
 * Replaced 'struct statfs' parameter with 'struct statvfs' in
 * version 2.5
 */
int bb_statfs(const char *path, struct statvfs *statv)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg("\nbb_statfs(path=\"%s\", statv=0x%08x)\n",
	    path, statv);
    bb_fullpath(fpath, path);

    // get stats for underlying filesystem
    retstat = 0;//log_syscall("statvfs", statvfs(fpath, statv), 0);

    log_statvfs(statv);

    return retstat;
}

/** Possibly flush cached data
 *
 * BIG NOTE: This is not equivalent to fsync().  It's not a
 * request to sync dirty data.
 *
 * Flush is called on each close() of a file descriptor.  So if a
 * filesystem wants to return write errors in close() and the file
 * has cached dirty data, this is a good place to write back data
 * and return any errors.  Since many applications ignore close()
 * errors this is not always useful.
 *
 * NOTE: The flush() method may be called more than once for each
 * open().  This happens if more than one file descriptor refers
 * to an opened file due to dup(), dup2() or fork() calls.  It is
 * not possible to determine if a flush is final, so each flush
 * should be treated equally.  Multiple write-flush sequences are
 * relatively rare, so this shouldn't be a problem.
 *
 * Filesystems shouldn't assume that flush will always be called
 * after some writes, or that if will be called at all.
 *
 * Changed in version 2.2
 */
// this is a no-op in BBFS.  It just logs the call and returns success
int bb_flush(const char *path, struct fuse_file_info *fi)
{
    log_msg("\nbb_flush(path=\"%s\", fi=0x%08x)\n", path, fi);
    // no need to get fpath on this one, since I work from fi->fh not the path
    log_fi(fi);


    return 0;
}

/** Release an open file
 *
 * Release is called when there are no more references to an open
 * file: all file descriptors are closed and all memory mappings
 * are unmapped.
 *
 * For every open() call there will be exactly one release() call
 * with the same flags and file descriptor.  It is possible to
 * have a file opened more than once, in which case only the last
 * release will mean, that no more reads/writes will happen on the
 * file.  The return value of release is ignored.
 *
 * Changed in version 2.2
 */
int bb_release(const char *path, struct fuse_file_info *fi)
{
    log_msg("\nbb_release(path=\"%s\", fi=0x%08x)\n",
	  path, fi);
    log_fi(fi);

    // We need to close the file.  Had we allocated any resources
    // (buffers etc) we'd need to free them here as well.
    return 0;
}

/** Synchronize file contents
 *
 * If the datasync parameter is non-zero, then only the user data
 * should be flushed, not the meta data.
 *
 * Changed in version 2.2
 */
int bb_fsync(const char *path, int datasync, struct fuse_file_info *fi)
{
    log_msg("\nbb_fsync(path=\"%s\", datasync=%d, fi=0x%08x)\n",
	    path, datasync, fi);
    log_fi(fi);

    // some unix-like systems (notably freebsd) don't have a datasync call
#ifdef HAVE_FDATASYNC
    if (datasync)
	return 0;//log_syscall("fdatasync", fdatasync(fi->fh), 0);
    else
#endif
	return 0;//log_syscall("fsync", fsync(fi->fh), 0);
}

#ifdef HAVE_SYS_XATTR_H
/** Note that my implementations of the various xattr functions use
    the 'l-' versions of the functions (eg bb_setxattr() calls
    lsetxattr() not setxattr(), etc).  This is because it appears any
    symbolic links are resolved before the actual call takes place, so
    I only need to use the system-provided calls that don't follow
    them */

/** Set extended attributes */
int bb_setxattr(const char *path, const char *name, const char *value, size_t size, int flags)
{
    char fpath[PATH_MAX];

    log_msg("\nbb_setxattr(path=\"%s\", name=\"%s\", value=\"%s\", size=%d, flags=0x%08x)\n",
	    path, name, value, size, flags);
    bb_fullpath(fpath, path);

    return 0;//log_syscall("lsetxattr", lsetxattr(fpath, name, value, size, flags), 0);
}

/** Get extended attributes */
int bb_getxattr(const char *path, const char *name, char *value, size_t size)
{
  //   int retstat = 0;
  //   char fpath[PATH_MAX];
  //
     log_msg("\nbb_getxattr(path = \"%s\", name = \"%s\", value = 0x%08x, size = %d)\n",
	     path, name, value, size);
  //   bb_fullpath(fpath, path);
  //
  //   retstat = log_syscall("lgetxattr", lgetxattr(fpath, name, value, size), 0);
  //   if (retstat >= 0)
	// log_msg("    value = \"%s\"\n", value);
  //
  //   return retstat;
  return 0;
}

/** List extended attributes */
int bb_listxattr(const char *path, char *list, size_t size)
{
    int retstat = 0;
    char fpath[PATH_MAX];
    char *ptr;

    log_msg("\nbb_listxattr(path=\"%s\", list=0x%08x, size=%d)\n",
	    path, list, size
	    );
    bb_fullpath(fpath, path);

    retstat = 0;//log_syscall("llistxattr", llistxattr(fpath, list, size), 0);
    if (retstat >= 0) {
	log_msg("    returned attributes (length %d):\n", retstat);
	if (list != NULL)
	    for (ptr = list; ptr < list + retstat; ptr += strlen(ptr)+1)
		log_msg("    \"%s\"\n", ptr);
	else
	    log_msg("    (null)\n");
    }

    return retstat;
}

/** Remove extended attributes */
int bb_removexattr(const char *path, const char *name)
{
    char fpath[PATH_MAX];

    log_msg("\nbb_removexattr(path=\"%s\", name=\"%s\")\n",
	    path, name);
    bb_fullpath(fpath, path);

    return 0;//log_syscall("lremovexattr", lremovexattr(fpath, name), 0);
}
#endif

/** Open directory
 *
 * This method should check if the open operation is permitted for
 * this  directory
 *
 * Introduced in version 2.3
 */
int bb_opendir(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg("\nbb_opendir(path=\"%s\", fi=0x%08x)\n",
	  path, fi);

    bb_fullpath(fpath, path);

    bb_file *dir = check_bbfile(path);
    if(dir == NULL){
        return -ENOENT;
    }
    if(!S_ISDIR(dir->metadata->mode)){
        return -ENOTDIR;
    }

    fi->fh = (intptr_t) dir;

    log_msg("\nopenDir Success\n");
    return retstat;
}

/** Read directory
 *
 * This supersedes the old getdir() interface.  New applications
 * should use this.
 *
 * The filesystem may choose between two modes of operation:
 *
 * 1) The readdir implementation ignores the offset parameter, and
 * passes zero to the filler function's offset.  The filler
 * function will not return '1' (unless an error happens), so the
 * whole directory is read in a single readdir operation.  This
 * works just like the old getdir() method.
 *
 * 2) The readdir implementation keeps track of the offsets of the
 * directory entries.  It uses the offset parameter and always
 * passes non-zero offset to the filler function.  When the buffer
 * is full (or an error happens) the filler function will return
 * '1'.
 *
 * Introduced in version 2.3
 */

int bb_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
	       struct fuse_file_info *fi)
{
    int retstat = 0;
    char fpath[PATH_MAX];
     log_msg("\nbb_readdir(path=\"%s\", buf=0x%08x, filler=0x%08x, offset=%lld, fi=0x%08x)\n",
	    path, buf, filler, offset, fi);
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    bb_fullpath(fpath, path);
	bb_file * dir_node = check_bbfile(path);

	if(dir_node == NULL){
		return -ENOENT;
	}
  if (!S_ISDIR(dir_node->metadata->mode)) {
      return -ENOTDIR;
  }
	else{
		for(int i = 0; i < dir_node->metadata->num_children; i++){
			log_msg(":%s:\n", dir_node->files[i].metadata->name);
			filler( buf, dir_node->files[i].metadata->name, NULL, 0 );
		}
	}
 log_msg("\nreadDir success\n");

  retstat = log_error("bb_readdir readdir");


    return retstat;
}


/** Release directory
 *
 * Introduced in version 2.3
 */
int bb_releasedir(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;

    log_msg("\nbb_releasedir(path=\"%s\", fi=0x%08x)\n",
	    path, fi);
    log_fi(fi);
    //closedir((DIR *) (uintptr_t) fi->fh);
    return retstat;
}

/** Synchronize directory contents
 *
 * If the datasync parameter is non-zero, then only the user data
 * should be flushed, not the meta data
 *
 * Introduced in version 2.3
 */
// when exactly is this called?  when a user calls fsync and it
// happens to be a directory? ??? >>> I need to implement this...
int bb_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi)
{
    int retstat = 0;

    log_msg("\nbb_fsyncdir(path=\"%s\", datasync=%d, fi=0x%08x)\n",
	    path, datasync, fi);
    log_fi(fi);

    return retstat;
}

/**
 * Initialize filesystem
 *
 * The return value will passed in the private_data field of
 * fuse_context to all file operations and as a parameter to the
 * destroy() method.
 *
 * Introduced in version 2.3
 * Changed in version 2.6
 */
// Undocumented but extraordinarily useful fact:  the fuse_context is
// set up before this function is called, and
// fuse_get_context()->private_data returns the user_data passed to
// fuse_main().  Really seems like either it should be a third
// parameter coming in here, or else the fact should be documented
// (and this might as well return void, as it did in older versions of
// FUSE).
void *bb_init(struct fuse_conn_info *conn)
{
    log_msg("\nbb_init()\n");

    log_conn(conn);
    log_fuse_context(fuse_get_context());

    //// OUR CODE::::
    //all_blocks = (bb_block *)calloc(MAX_FILES*MAX_BLOCKS, sizeof(bb_block));
    all_blocks = (bb_all_blocks *)calloc(1, sizeof(bb_all_blocks));
    root_files = (bb_file *)malloc(sizeof(bb_file));
    root_files->metadata = (bb_metadata *)malloc(sizeof(bb_metadata));

    if (root_files == NULL || all_blocks == NULL){
        printf("\nerror malloc\n");
        return NULL;
    }

    //initialise fields
    root_files->metadata->size = 0;
    strcpy(root_files->metadata->name,"/");
    root_files->metadata->num_block = 0;
    root_files->metadata->num_children = 0;
    root_files->files = (bb_file *)malloc(sizeof(bb_file));
    strcpy(root_files->metadata->fullpath,"/");
    //root_files->block = NULL;//its a dir, no need for blocks
    root_files->metadata->mode = S_IRWXO | S_IRWXG | S_IRWXU | S_IFDIR;

    all_blocks->num_blocks = 0;
    //for (int i = 0; i< MAX_FILES*MAX_BLOCKS; i++)
    all_blocks->blocks = (bb_block *)malloc(sizeof(bb_block));
    strcpy(all_blocks->blocks->hash, "00000000000000000000");

    return BB_DATA;
}

/**
 * Clean up filesystem
 *
 * Called on filesystem exit.
 *
 * Introduced in version 2.3
 */
void bb_destroy(void *userdata)
{

    log_msg("\nbb_destroy(userdata=0x%08x)\n", userdata);
    bb_file *cur = root_files;

    free(cur->files);
    free(cur);
    free(all_blocks->blocks);
    free(all_blocks);

}

/**
 * Check file access permissions
 *
 * This will be called for the access() system call.  If the
 * 'default_permissions' mount option is given, this method is not
 * called.
 *
 * This method is not called under Linux kernel versions 2.4.x
 *
 * Introduced in version 2.5
 */
int bb_access(const char *path, int mask)
{
  //   int retstat = 0;
  //   char fpath[PATH_MAX];
  //
     log_msg("\nbb_access(path=\"%s\", mask=0%o)\n",
	     path, mask);
  //   bb_fullpath(fpath, path);
  //
  //   retstat = access(fpath, mask);
  //
  //   if (retstat < 0)
	// retstat = log_error("bb_access access");
  //
  //   return retstat;
  return 0;
}

/**
 * Create and open a file
 *
 * If the file does not exist, first create it with the specified
 * mode, and then open it.
 *
 * If this method is not implemented or under Linux kernel
 * versions earlier than 2.6.15, the mknod() and open() methods
 * will be called instead.
 *
 * Introduced in version 2.5
 */
// Not implemented.  I had a version that used creat() to create and
// open the file, which it turned out opened the file write-only.

/**
 * Change the size of an open file
 *
 * This method is called instead of the truncate() method if the
 * truncation was invoked from an ftruncate() system call.
 *
 * If this method is not implemented or under Linux kernel
 * versions earlier than 2.6.15, the truncate() method will be
 * called instead.
 *
 * Introduced in version 2.5
 */
int bb_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi)
{
    int retstat = 0;

    log_msg("\nbb_ftruncate(path=\"%s\", offset=%lld, fi=0x%08x)\n",
	    path, offset, fi);
    log_fi(fi);

    retstat = ftruncate(fi->fh, offset);
    if (retstat < 0)
	retstat = log_error("bb_ftruncate ftruncate");

    return retstat;
}

/**
 * Get attributes from an open file
 *
 * This method is called instead of the getattr() method if the
 * file information is available.
 *
 * Currently this is only called after the create() method if that
 * is implemented (see above).  Later it may be called for
 * invocations of fstat() too.
 *
 * Introduced in version 2.5
 */
int bb_fgetattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi)
{
    int retstat = 0;

    log_msg("\nbb_fgetattr(path=\"%s\", statbuf=0x%08x, fi=0x%08x)\n",
	    path, statbuf, fi);
    log_fi(fi);

    // On FreeBSD, trying to do anything with the mountpoint ends up
    // opening it, and then using the FD for an fgetattr.  So in the
    // special case of a path of "/", I need to do a getattr on the
    // underlying root directory instead of doing the fgetattr().
    if (!strcmp(path, "/"))
	return bb_getattr(path, statbuf);

    retstat = 0;//fstat(fi->fh, statbuf);
    if (retstat < 0)
	retstat = log_error("bb_fgetattr fstat");

    log_stat(statbuf);

    return retstat;
}

struct fuse_operations bb_oper = {
  .getattr = bb_getattr,
  .readlink = bb_readlink,
  // no .getdir -- that's deprecated
  .getdir = NULL,
  .mknod = bb_mknod,
  .mkdir = bb_mkdir,
  .unlink = bb_unlink,
  .rmdir = bb_rmdir,
  .symlink = bb_symlink,
  .rename = bb_rename,
  .link = bb_link,
  .chmod = bb_chmod,
  .chown = bb_chown,
  .truncate = bb_truncate,
  .utime = bb_utime,
  .open = bb_open,
  .read = bb_read,
  .write = bb_write,
  /** Just a placeholder, don't set */ // huh???
  .statfs = bb_statfs,
  .flush = bb_flush,
  .release = bb_release,
  .fsync = bb_fsync,

#ifdef HAVE_SYS_XATTR_H
  .setxattr = bb_setxattr,
  .getxattr = bb_getxattr,
  .listxattr = bb_listxattr,
  .removexattr = bb_removexattr,
#endif

  .opendir = bb_opendir,
  .readdir = bb_readdir,
  .releasedir = bb_releasedir,
  .fsyncdir = bb_fsyncdir,
  .init = bb_init,
  .destroy = bb_destroy,
  .access = bb_access,
  .ftruncate = bb_ftruncate,
  .fgetattr = bb_fgetattr
};

void bb_usage()
{
    fprintf(stderr, "usage:  bbfs [FUSE and mount options] rootDir mountPoint\n");
    abort();
}

int main(int argc, char *argv[])
{
    int fuse_stat;
    struct bb_state *bb_data;

    // bbfs doesn't do any access checking on its own (the comment
    // blocks in fuse.h mention some of the functions that need
    // accesses checked -- but note there are other functions, like
    // chown(), that also need checking!).  Since running bbfs as root
    // will therefore open Metrodome-sized holes in the system
    // security, we'll check if root is trying to mount the filesystem
    // and refuse if it is.  The somewhat smaller hole of an ordinary
    // user doing it with the allow_other flag is still there because
    // I don't want to parse the options string.
    if ((getuid() == 0) || (geteuid() == 0)) {
    	fprintf(stderr, "Running BBFS as root opens unnacceptable security holes\n");
    	return 1;
    }

    // See which version of fuse we're running
    fprintf(stderr, "Fuse library version %d.%d\n", FUSE_MAJOR_VERSION, FUSE_MINOR_VERSION);

    // Perform some sanity checking on the command line:  make sure
    // there are enough arguments, and that neither of the last two
    // start with a hyphen (this will break if you actually have a
    // rootpoint or mountpoint whose name starts with a hyphen, but so
    // will a zillion other programs)
    if ((argc < 3) || (argv[argc-2][0] == '-') || (argv[argc-1][0] == '-'))
	bb_usage();

    bb_data = malloc(sizeof(struct bb_state));
    if (bb_data == NULL) {
	perror("main calloc");
	abort();
    }

    // Pull the rootdir out of the argument list and save it in my
    // internal data
    bb_data->rootdir = realpath(argv[argc-2], NULL);
    argv[argc-2] = argv[argc-1];
    argv[argc-1] = NULL;
    argc--;

    bb_data->logfile = log_open();

    // turn over control to fuse
    fprintf(stderr, "about to call fuse_main\n");
    //struct fuse_operations bb_oper = {0};
    fuse_stat = fuse_main(argc, argv, &bb_oper, bb_data);
    fprintf(stderr, "fuse_main returned %d\n", fuse_stat);

    return fuse_stat;
}
