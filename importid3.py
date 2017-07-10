#!/usr/bin/python

import shutil, os
from taglib import tagopen, InvalidMedia, ValidationError
import sqlite3
import StringIO

shutil.copyfile('../db/media.sqlite', '../db/media_snapshot2.sqlite')

tracks = {}
sql_read = sqlite3.connect('../db/media_snapshot2.sqlite')
sql_write = sqlite3.connect('../db/media.sqlite')
sql_img = sqlite3.connect('../db/media.sqlite')
sql_img.text_factory = str
for (id, path) in sql_read.execute("SELECT id, path FROM track JOIN track_path_fingerprint ON track.fingerprint = track_path_fingerprint.fingerprint"):
	print path
	try:
		tag = tagopen(path)
		if (tag.image):
			sql_img.execute("INSERT OR IGNORE INTO track_img (track_id, img, format, source) VALUES (?, ?, 'raw', 'id3')", (id, tag.image.tostring()))
			sql_img.commit()
		if (tag.name):
			sql_write.execute("INSERT OR IGNORE INTO track_tags (track_id, tag_id, value, source) VALUES (?, ?, ?, 'id3')", (id, 4,tag.name));
		if (tag.album):
			sql_write.execute("INSERT OR IGNORE INTO track_tags (track_id, tag_id, value, source) VALUES (?, ?, ?, 'id3')", (id, 8,tag.album));
		if (tag.artist):
			sql_write.execute("INSERT OR IGNORE INTO track_tags (track_id, tag_id, value, source) VALUES (?, ?, ?, 'id3')", (id, 5,tag.artist));
		if (tag.year):
			sql_write.execute("INSERT OR IGNORE INTO track_tags (track_id, tag_id, value, source) VALUES (?, ?, ?, 'id3')", (id, 19,tag.year));
		sql_write.commit()
	except InvalidMedia:
		print 'no decoder found'

os.remove('../db/media_snapshot2.sqlite')
