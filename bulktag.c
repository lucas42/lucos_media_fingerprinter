#include <stdio.h>
#include <sqlite3.h>
#include <ftw.h>
#include <stdlib.h>
#include <string.h>

sqlite3 *db;
int force = 0;
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

int add_tag(int trackid, char *key, char *val) {
	
	
	int rc;
	sqlite3_stmt *stmt;
	int tagid;
	
	// Check if the tag is in the db.
	sqlite3_prepare_v2(db, "SELECT id FROM tag WHERE label = ?", -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, key, -1, SQLITE_TRANSIENT);
	sqlite3_step(stmt);
	tagid = (int)sqlite3_column_int(stmt, 0);
	
	// If not, add it
	if (!tagid) {
		sqlite3_prepare_v2(db, "INSERT INTO tag (label) VALUES (?)", -1, &stmt, 0);
		sqlite3_bind_text(stmt, 1, key, -1, SQLITE_TRANSIENT);
		sqlite3_step(stmt);
		
		sqlite3_prepare_v2(db, "SELECT id FROM tag WHERE label = ?", -1, &stmt, 0);
		sqlite3_bind_text(stmt, 1, key, -1, SQLITE_TRANSIENT);
		sqlite3_step(stmt);
		tagid = (int)sqlite3_column_int(stmt, 0);
	}
	printf("\tAdding tag:%s=>%s (tagid = %d)\n", key, val, tagid);
	
	char *query;
	if (force) query = "REPLACE INTO track_tags (track_id, tag_id, value, source) VALUES (?, ?, ?, ?)";
	else query = "INSERT OR IGNORE INTO track_tags (track_id, tag_id, value, source) VALUES (?, ?, ?, ?)";
	
	rc = sqlite3_prepare_v2(db, query,-1,&stmt,0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL [1] prepare error: %d\n", rc);
		fprintf(stderr, "SQL [1] ERROR: %s\n", sqlite3_errmsg(db));
	} else {
		sqlite3_bind_int(stmt, 1, trackid);
		sqlite3_bind_int(stmt, 2, tagid);
		sqlite3_bind_text(stmt, 3, val, -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 4, "bulk", -1, SQLITE_TRANSIENT);
		rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE) {
			fprintf(stderr, "SQL [1] step error: %d\n", rc);
			fprintf(stderr, "SQL [1] ERROR: %s\n", sqlite3_errmsg(db));
		}
	}
	return 0;
}


int num_tags = 0;
char **tags;
int tag_file(const char *file_name, const struct stat *status, int type) {
	
	// If not a file, then ignore
	if (type != FTW_F) {
		return 0;
	}
	sqlite3_stmt *stmt;
	int trackid;
	
	// Check if the file is in the db.
	sqlite3_prepare_v2(db, "SELECT track.id FROM track JOIN track_path_fingerprint ON track.fingerprint = track_path_fingerprint.fingerprint WHERE path = ?", -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, file_name, -1, SQLITE_TRANSIENT);
	sqlite3_step(stmt);
	trackid = (int)sqlite3_column_int(stmt, 0);
	
	// If there's no track id then ignore
	if (!trackid) {
		printf("Can't find track:%s, ignoring\n", file_name);
		return 0;
	}
	printf("Tagging file:%s (track id = %d)\n", file_name, trackid);
	
	int ii;
	char *val, *key;
	for (ii = 0; ii < num_tags; ii++) {
		val = strstr(tags[ii], "=");
		int keylen = strlen(tags[ii]) - strlen(val);
		val++;
		key= malloc((keylen+1) * sizeof(char *));
		strncpy(key, tags[ii], keylen);
		key[keylen] = '\0';
		add_tag(trackid, key, val);
	}
	return 0;
}


int main(int argc, char **argv)
{
	int ii, num_file_names = 0;
	char **file_names;
	char actualpath [PATH_MAX+1];
	
	file_names = malloc(argc * sizeof(char *));
	tags = malloc(argc * sizeof(char *));
	for (ii = 1; ii < argc; ii++) {
		char *arg = argv[ii];
		if (!strcmp(arg, "-f")) {
			force = 1;
		} else if (!strcmp(arg, "-t")) {
			if (++ii < argc && strstr(argv[ii], "=")) {
				tags[num_tags++] = argv[ii];
			} else {
				printf("Warning: tag %s has no value, ignoring\n", argv[ii]);
			}
		} else {
			file_names[num_file_names++] = argv[ii];
		}
	}

	if (!num_file_names || !num_tags) {
		printf("usage: %s OPTIONS FILE...\n\n", argv[0]);
		printf("Options:\n");
		printf("  -t KEY=VAL a tag to attach to the given files.\n");
		printf("  -f Overwrite existing values in the database.\n");
		return 2;
	}
	
	sqlite3_open("/web/lucos/media/db/media.sqlite", &db);

	for (ii = 0; ii < num_file_names; ii++) {
		realpath(file_names[ii], actualpath);
		printf("File=%s\n", actualpath);
		ftw(actualpath, tag_file, 1);
	}
	
	free(file_names);
	free(tags);

    sqlite3_close(db);

	return 1;
}
