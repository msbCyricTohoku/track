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
#define KCYN "\x1B[36m"

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
    mkdir(dst, 0755);

    DIR *d = opendir(src);
    if (!d)
      return;

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 ||
          strcmp(entry->d_name, ".track") == 0)
        continue; // le jump :D

      char new_src[1024];
      char new_dst[1024];

      snprintf(new_src, sizeof(new_src), "%s/%s", src, entry->d_name);
      snprintf(new_dst, sizeof(new_dst), "%s/%s", dst, entry->d_name);

      copy_recursive(new_src, new_dst);
    }
    closedir(d);
  } else {
    copy_file(src, dst);
  }
}

// lists all snapshots inside .track
void list_log() {
  DIR *d = opendir(".track");
  if (!d) {
    printf(KRED "No tracks found. Try tracking something first!\n" KNRM);
    return;
  }

  printf(KCYN "--- TRACK HISTORY ---\n" KNRM);
  struct dirent *entry;
  int count = 0;
  while ((entry = readdir(d)) != NULL) {
    if (entry->d_name[0] == '.')
      continue; // skip hidden files
    printf(" Snapshot: " KMAG "%s" KNRM "\n", entry->d_name);
    count++;
  }
  if (count == 0)
    printf("Nothing tracked yet.\n");
  closedir(d);
}

// compares current dir with specific snapshot using system diff
void diff_snapshot(const char *timestamp) {
  char cmd[2048];
  char path[512];

  snprintf(path, sizeof(path), ".track/%s", timestamp);

  struct stat st;
  if (stat(path, &st) != 0) {
    printf(KRED "Error: Snapshot %s does not exist.\n" KNRM, timestamp);
    return;
  }

  printf(KCYN "Comparing current directory with snapshot: %s\n" KNRM,
         timestamp);
  printf("--------------------------------------------------\n");

  // diff posix, excluse .track folder
  snprintf(cmd, sizeof(cmd), "diff -r -u --exclude=\".track\" %s .", path);
  system(cmd);
}

int main(int argc, char *argv[]) {
  // logic to handle commands: log and diff
  if (argc > 1) {
    if (strcmp(argv[1], "log") == 0) {
      list_log();
      return 0;
    } else if (strcmp(argv[1], "diff") == 0) {
      if (argc < 3) {
        printf(KRED "Usage: %s diff <timestamp_folder>\n" KNRM, argv[0]);
        return 1;
      }
      diff_snapshot(argv[2]);
      return 0;
    }
  }

  // create a snapshot
  time_t rawtime;
  struct tm *timeinfo;
  char timestamp[20];
  char dest[512];

  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d_%H%M%S", timeinfo);

  mkdir(".track", 0755);
  snprintf(dest, sizeof(dest), ".track/%s", timestamp);

  if (mkdir(dest, 0755) != 0) {
    fprintf(stderr, KRED "Error: Could not create backup directory %s\n" KNRM,
            dest);
    return 1;
  }

  printf(KMAG "--------------------------------------\n" KNRM);
  printf(KMAG "|      TRACK V0.1 Developed by msb    |\n" KNRM);
  printf(KMAG "--------------------------------------\n" KNRM);

  printf("Tracking to: %s\n", dest);
  copy_recursive(".", dest);

  printf(KBLU "You tracked! snapshot created Boss!!!\n" KNRM);

  return 0;
}
