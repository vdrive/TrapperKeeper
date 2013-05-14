========================================================================
    MICROSOFT FOUNDATION CLASS LIBRARY : BTTorrentGenerator Project Overview
========================================================================

The BTTorrentGenerator project is used to generate the bit torrent files.
It takes input from the database bittorrent_data and outputs the the
torrent files in the C:\\BTTorrentGenerator directory. It also outputs 
diagnostic log messages to that directory. It updates the database tables with 
information on the new torrents generated.

Requirements:
-------------
1. A decoy data file called "TorrentDecoyFile" must be present in the
TrapperKeeper\BTTorrentGenerator directory. This is usually a large file
that provides the decoy data for the torrents. This same file also
has to be present in the TrapperKeeper\BitTorrentClient directory for
the Client to generate the decoy data to send to the requesting 
BitTorrent clients for the torrents we are seeding.

2. Database tables:
------------------
a)    bt_generator_maininfo: The torrent that needs to be generated has to
be input into this table. The table has an auto increment id that is
automatically generated for each input torrent file. This auto_torrent_id
is to be used as the torrent_id for the tables bt_generator_datafile,
bt_generator_pieces, bt_generator_seedless_pieces.
	The columns to be input are: 
	torrentfilename :the name of the torrent file to be generated.
	topmost_file_or_dir: the name of the directory to contain the 
	datafiles (for multiple data files) OR the name of the 
	datafile.
	piece_length: the piece_length of each piece in the torrent. 
	(Default is 262144 bytes).
The info_hash, creation_date and total_length are output by the
BTTorrentGenerator.

b)    bt_generator_datafile: This table contains information on the
different datafiles of a torrent. The fields input are:
	torrent_id: same as auto_torrent_id of the torrent from 
	the bt_generator_maininfo table.
	file_name: name of datafile.
	file_length: lenght in bytes of datafile.
	md5sum: (optional) md5 sum of file.
	file_number: The order in which the datafile has to 
	appear in the "info" dictionary of the torrent.

c)   bt_generator_datapath: Information on the directory hierachy
of the datafile in the torrent. The fields to be input are:
	file_name: name of file.
	directory_name: name of directory.
	hierarchy_number: the hierarchy number of the directory in the
	torrent.
example: for a torrent with:
dir1/dir2/file1.mp3
file1.mp3 will have 2 entries, one for each dir1 and dir2. The hierarchy
number of dir1 is 1 and that of dir2 is 2.

d)    bt_generator_announcelist: Contains the announce URLs of the 
trackers for the torrent. The fileds to be input are:
	torrent_id: id of the torrent.
	tracker_number: the order number of the tracker. (number 1 is
	the announce URL, and number 2,3,... are part of the announce
	list.)
	tracker_url: url of the tracker.

e)    bt_generator_pieces: This is a table output by BTTorrentGenerator.
The fields are:
	torrent_id: id of torrent.
	piece_num: number of the piece.
	piece_hash: the SHA1 hash of the data piece.

f)    bt_generator_seedless_pieces: This lists the pieces of the torrent
that are not to be seeded by the BitTorrentClient. It is output by the
BTTorrentGenerator, only if there is no existing entry for a torrent. It
is a set of randomly generated pieces (1% to 20% of the total pieces.)
The fields output are:
	torrent_id: id of torrent.
	piece_num: The piece number that is not to be seeded.

