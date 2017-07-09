#!/usr/bin/python

import sqlite3
import json
import urllib, urllib2

apikey = ''

# Warning: overwrites existing metadata in db

sql = sqlite3.connect('../db/media.sqlite')
for row in sql.execute("SELECT id,fingerprint, duration FROM track WHERE id > 14777 AND fingerprint <> 'AQAAAA'"):
	(id, fingerprint, duration) = row
	url = 'http://api.acoustid.org/v2/lookup'
	values = {'format': 'json', 'client': apikey, 'duration': duration, 'fingerprint': fingerprint, 'meta': 2}
	data = urllib.urlencode(values)
	req = urllib2.Request(url, data)
	httpresponse = urllib2.urlopen(req)
	response = json.loads(httpresponse.read())

	if len(response['results']) > 1:
		try:
			data = {}
			data['acoustid'] = response['results'][0]['id']
			data['mbid_recording'] = response['results'][0]['recordings'][0]['id']
			data['title'] = response['results'][0]['recordings'][0]['tracks'][0]['title']
			data['artist'] = response['results'][0]['recordings'][0]['tracks'][0]['artist']['name']
			data['mbid_artist'] = response['results'][0]['recordings'][0]['tracks'][0]['artist']['id']
			data['mbid_release'] = response['results'][0]['recordings'][0]['tracks'][0]['medium']['release']['id']
			data['album'] = response['results'][0]['recordings'][0]['tracks'][0]['medium']['release']['title']

			for label in data:
				value = data[label]
				sql.execute("REPLACE INTO track_tags (track_id, tag_id, value, source) SELECT ?, id, ?, ? FROM tag WHERE label = ?", (id, value, label, 'acoustid'))
			sql.commit()
		except:
			print "exception"
	print id
