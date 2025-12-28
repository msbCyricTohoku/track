// track
// simple dir. tracking, alternative to git using internal copy logic
// offers super fast snapshot
// by msb -- Dec 2025

#include <dirent.h>   // opendir, readdir, closedir
#include <fcntl.h>    // open
#include <stdio.h>    // printf
#include <stdlib.h>   // system
#include <string.h>   // strcat, strcpy, strcmp
#include <sys/stat.h> // stat, chmod, mkdir
#include <time.h>     // time, localtime, strftime
#include <unistd.h>   // read, write, close

#define KNRM "\x1B[0m"
#define KMAG "\x1B[35m"
#define KBLU "\x1B[34m"
#define KRED "\x1B[31m"

void copy_file(const char *src, const char *dst) {
  int in = open(src, O_RDONLY);
  if (in < 0)
    return;

  int out = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0777);
  if (out < 0) {
    close(in);
    return;
  }

  char buffer[64 * 1024]; // set buffer to 64 KiB
  ssize_t r;

  while ((r = read(in, buffer, sizeof(buffer))) > 0) {
    ssize_t off = 0;
    while (off < r) {
      ssize_t byteR = r - off;
      // printf("bytes write %zd\n", off);
      off += write(out, buffer + off, byteR);
    }
  }

  close(in);
  close(out);
  chmod(dst, 0644);
}

void copy_recursive(const char *src, const char *dst) {
  struct stat st;
  if (stat(src, &st) != 0)
    return;

  if (S_ISDIR(st.st_mode)) {
    // create the destination directory
    mkdir(dst, 0755); // using 0755 for directories not sure if 0644 is better

    DIR *d = opendir(src);
    if (!d)
      return;

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
      // skip "." and ".." AND skip the ".track" folder itself
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 ||
          strcmp(entry->d_name, ".track") == 0)
        continue; // le jump :D, next iter

      char new_src[1024];
      char new_dst[1024];

      printf("Tracking %s\n", entry->d_name);

      snprintf(new_src, sizeof(new_src), "%s/%s", src, entry->d_name);
      snprintf(new_dst, sizeof(new_dst), "%s/%s", dst, entry->d_name);

      copy_recursive(new_src, new_dst);
    }
    closedir(d);
  } else {
    copy_file(src, dst);
  }
}

int main() {
  time_t rawtime;
  struct tm *timeinfo;
  char timestamp[20];
  char dest[512];

  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d_%H%M%S", timeinfo);

  // ensure the base .track directory exists
  mkdir(".track", 0755);

  snprintf(dest, sizeof(dest), ".track/%s", timestamp);

  // create the specific snapshot directory
  if (mkdir(dest, 0755) != 0) {
    fprintf(stderr, KRED "Error: Could not create backup directory %s\n" KNRM,
            dest);
    return 1;
  }

  printf(KMAG "--------------------------------------\n" KNRM);
  printf(KMAG "|     TRACK V0.1 Developed by msb    |\n" KNRM);
  printf(KMAG "--------------------------------------\n" KNRM);

  printf("Tracking to: %s\n", dest);

  // skip .track inside copy_recursive to avoid recursion loops
  copy_recursive(".", dest);

  printf(KBLU "You tracked! snapshot created Boss!!!\n" KNRM);

  return 0;
}
