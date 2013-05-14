#include "StdAfx.h"
#include "torrentspyposter.h"
#include "categories.h"

TorrentSpyPoster::TorrentSpyPoster(void)
{
	m_main_category = "4";
	char * names = ("Applications","Games","Music","Movies","TV","Anime","Hentai","Linux","Macintosh","Misc","Adult","Unsorted/Other");
	char * ids = ("1","2","3","4","5","6","7","8","9","10","11","12");
	vector<Categories> v_cat;
	names =("Handheld","Multimedia","Plugins","Programming","Screensaver","Security","Tools","Unsorted","Wallpaper","Windows");
	ids = ("7","2","6","5","542","4","3","8","543","1");
	v_cat.push_back(Categories(names, ids));
	// games
	names = ("Dreamcast","GameCube","PC","Pocket PC","PS 2","PSX","ROMS","Sega Saturn","Unsorted","XBox");
	ids = ("12","13","9","295","10","15","16","14","17","11");
	v_cat.push_back(Categories(names, ids));
	// music
	names = ("Alternative","Anime Mus..Vids","Asian","Blues","Christian","Christmas","Classic","Country/Western","Drum N Bass","Electronic","Game Music","Gothic","Hardcore","Hip Hop","House","Indie","Industrial","International/World","Japan Music..Vids","Japan Pop","Jazz","Karaoke","Live Music/Concerts","Metal","Music Video\"s","Music VIdeos - Live/Concerts","Pop","Progressive Rock","Punk","R&B","Rap","Reggae","Rock","Russian","Ska","Soundtracks","Spanish","Techno","Trance","Trip Hop","Unsigned/Amateur","Unsorted");
	ids =("18","325","339","19","20","21","22","23","24","25","26","27","345","28","343","29","30","381","327","324","31","32","386","33","34","583","35","390","36","37","38","39","40","41","42","43","44","45","337","342","46","47");
	v_cat.push_back(Categories(names, ids));
	// movies
	names = ("Action","Action - Martial Arts","Adventure","Animation","Biography","Comedy","Concerts","Conspiracy Theory","Crime","Documentary","Drama","Fantasy","Film-Noir","Film/DVD - Clips","Film/DVD - Extras","Film/DVD - Trailers","Format - DVD-R","Format - for Handheld","Format - ratDVD","Format - SVCD","History","Horror","Kids/Family","Music","Musical","Mystery","New - CAM","New - SCR (DVD/VHS Screener)","New - TC (TeleCine)","New - TS (TeleSync)","Non-English - Asian","Non-English - Danish","Non-English - Dutch","Non-English - English dub","Non-English - English sub","Non-English - Finnish","Non-English - French","Non-English - German","Non-English - Greek","Non-English - Hindi","Non-English - Italian","Non-English - Japanese","Non-English - Multi-Lingual","Non-English - Nordic","Non-English - Norwegian","Non-English - Polish","Non-English - Portuguese","Non-English - Romanian","Non-English - Russian","Non-English - Spanish","Non-English - Swedish","Non-English - Unsorted","Non-imdb - Automotive","Non-imdb - Educational","Non-imdb - Sport","Non-imdb - Unsorted","Romance","Sci-Fi","Short","Sport","Thriller","Unsorted","War","Western");
	ids =("48","65","49","50","549","53","54","586","550","55","56","58","551","329","57","67","330","578","577","332","552","62","64","553","554","66","333","576","581","575","51","559","560","561","562","563","59","60","565","61","63","566","567","568","569","570","571","572","69","71","573","298","52","312","582","574","68","70","555","72","73","74","556","557");
	v_cat.push_back(Categories(names, ids));
	// tv
	names = ("21 Jump Street","24","3rd Rock from the Sun","7 Days","7th Heaven","8 Simple Rules","A Bike Is Born","A Racing Car Is Born","A Touch of Frost","Absolutely Fabulous","According to Jim","AdultSwim","ALF","Alias","All in the Family","Allo Allo","American Chopper","American Dad","American Dreams","American Gothic","American Idol","Americas Next Top Model","Andromeda","Angel","Aqua Teen","Arrested Development","Automan","Babylon 5","Bad Girls","Band of Brothers","Battlestar Galactica","Beavis and Butthead","Beverly Hillbillies","Beverly Hills","Big Brother","Black Adder","Black Books","Blakes 7","Blind Justice","Blue Collar","Boston Legal","Boston Public","Brimstone","Brisco County Jr","Buck Rogers","Buffy","C.S.I Miami","C.S.I New York","C.S.I. (Las Vegas)","Canadian Idol","Carnivale","Century City","Chappelle Show","Charlie Jade","Charmed","City of Men","Code Name Eternity","Cold Case","Combat","Comedy Central Presents","Commited","Con","Coronation Street","Coupling","Crime Traveller","Crossing Jordan","Curb Your Enthusiasm","Da Ali G Show","Daria","Dark Angel","Dark Shadows","Dawsons Creek","Dead Like Me","Dead Zone","Deadwood","Desperate Housewives","Dharma & Greg","Dilbert","Dirty Sanchez","Dr Who","Drawn Together","Drew Carey Show","Due South","E.R.","Earth 2","Earth Final Conflict","Earthsea","Eastenders","Eurovision Song Contest","Everwood","Everybody Loves Raymond","Extreme Engineering","Extreme Makeover HE","Eyes","Family Guy","Farscape","Father of the Pride","Fawlty Towers","Fifth Gear","Firefly","Forever Knight","Frasier","Freaks and Geeks","Fresh Prince of Bel Air","Friends","Futurama","Game On","Game Over","George Lopez","Gilligans Island","Gilmore Girls","Good Eats","Greatest American Hero","Grey\"s Anatomy","Happy Tree Friends","Hercules","History Bites","Hitchhiker\"s Guide","House","Huff","I Claudius","In Living Color","Iron Chef","Jack and Bobby","Jackass","JAG","Jake 2.0","Jake in Progress","Jeremiah","Joan of Arcadia","Joey","John Doe","Johnny Zero","Judging Amy","Kevin Hill","Kids in the Hall","King of Queens","Kingdom Hospital","Kojak","Kung Fu","Las Vegas","Late Night Conan O\"Brian","Late Show David Letterman","Laurel and Hardy","Law & Order","Little Britain","Living with Fran","Logans Run","Lost","MacGyver","Mad About You","Malcolm in the Middle","Married with Children","MASH","McLeod\"s Daughters","Medical Investigation","Medium","Meet the Barkers","Men Behaving Badly","Miami Vice","Midsomer Murders","Millennium","Misc - Action/Adventure","Misc - Biographies","Misc - British Comedy","Misc - British Drama","Misc - Cartoons/Animation","Misc - Children","Misc - Comedy","Misc - Commercials","Misc - Conspiracy/Paranormal","Misc - Cooking","Misc - Detective/Mystery","Misc - DIY/Home Improvement","Misc - Documentaries","Misc - Drama","Misc - Educational/Science","Misc - Game Show","Misc - Music","Misc - News","Misc - Poker","Misc - Politics","Misc - Reality TV","Misc - Religion","Misc - Sci-Fi","Misc - Sitcom","Misc - Stand-up Comedy","Misc - Talk Show","Misc - Variety","Misc - Western","Modern Marvels","Monk","Monster Garage","Monty Python","Mr Bean","Mr Show","Murder She Wrote","Mutant X","My Restaurant Rules","My Wife and Kids","Mystery Science Theatre","Mythbusters","NCIS","NewsRadio","Nikita","Ninja Turtles","Nip/Tuck","Non-English - Asian","Non-English - Danish","Non-English - Dutch","Non-English - English Sub","Non-English - Finnish","Non-English - French","Non-English - German","Non-English - Greek","Non-English - Italian","Non-English - Japanese","Non-English - Norwegian","Non-English - Polish","Non-English - Portuguese","Non-English - Romanian","Non-English - Spanish","Non-English - Swedish","Non-English - Tagalog","Non-English - Unsorted","Nowhere Man","Numb3rs","NYPD Blue","Odyssey 5","Off Centre","One Tree Hill","Only Fools and Horses","Outer Limits","OZ","Parker Lewis","Penn and Teller","Pilot Guides","Pimp My Ride","Point Pleasant","Poirot","Police Squad!","Power Rangers","Punk\"d","Quantum Leap","Queer As Folk","Queer Eye","Real Time with Bill Maher","Red Dwarf","ReGenesis","Reilly: Ace of Spies","Revelations","Rodney","Roswell","Saturday Night Live","Saved by The Bell","Scare Tactics","Scrubs","Seaquest","Secret Adv. of Jules Verne","Seinfeld","Sex & the City","Shameless","Sifl and Olly Show","Sir Arthur Doyle - Lost World","Six Feet Under","Sledge Hammer","Sliders","Smallville","Sopranos","South Park","Space Abov..Beyond","Spooks","Sports - Auto Racing","Sports - Baseball","Sports - Basketball","Sports - Bicycling","Sports - Boxing","Sports - Football (American)","Sports - Hockey","Sports - Martial Arts","Sports - Motorcycle Racing","Sports - Rugby","Sports - Soccer (Football)","Sports - Tennis","Sports - Unsorted","Sports - Wrestling","Sports Night","Stacked","Star Trek - DS9","Star Trek - Ent..","Star Trek - TNG","Star Trek - TOS","Star Trek - Voy..","Stargate Atlantis","Stargate SG1","Starhunter","Still Standing","SummerLand","Sunset Beach","Survivor","Taken","Tech TV","That 70\"s Show","The 4400","The Agency","The Amazing Race","The Apprentice","The Contender","The Daily Show (Jon Stewart)","The Fast Show","The Howard Stern Show","The L Word","The League of Gentlemen","The Lone Gunmen","The Muppet Show","The O.C.","The Office","The Pretender","The Prisoner","The Scene","The Shield","The Simple Life","The Simpsons","The Singing Detective","The Tomorrow People","The Tonight Show","The Tribe","The Tripods","The West Wing","The Wire","The Wonder Years","Third Watch","Threat Matrix","Tilt","Time Trax","Top Gear","Touching Evil","Trailer Park Boys","Trigger Happy TV","Tripping the Rift","Tru Calling","Twilight Zone","Twin Peaks","Two and Half Men","Two Guys and a Girl","UFO","Ultimate Force","Unsorted","V","Veronica Mars","Viva La Bam","Voyagers","Weird Science","What I Like About You","Whose Line Is It","Wildboyz","Will and Grace","Witchblade","Without a Trace","Wonder Showzen","Wonderfalls","World Series of Poker","X-Files","Xena","Zoey");
	ids =("419","76","420","77","354","78","421","422","423","79","424","315","426","80","81","427","389","340","82","83","84","428","85","86","87","88","90","91","92","392","93","94","359","429","95","397","316","96","398","373","355","299","97","98","430","99","399","400","100","477","101","103","104","431","105","432","106","107","338","370","344","433","108","109","110","401","111","434","435","436","113","114","115","116","117","346","437","334","438","384","365","119","120","121","122","123","124","125","480","127","375","439","402","440","128","129","376","301","393","130","131","132","442","366","133","134","443","135","444","136","137","138","139","403","140","141","142","404","349","336","445","143","144","353","405","145","146","383","147","148","314","377","352","380","382","149","150","151","406","152","153","368","369","154","155","407","446","156","303","157","481","158","159","447","448","378","356","449","450","451","452","160","544","453","478","479","102","482","483","454","545","455","484","485","118","486","126","456","457","487","394","546","318","547","458","488","371","548","489","228","459","372","161","162","163","460","408","164","165","367","166","341","357","490","491","167","348","89","492","461","313","462","463","464","493","494","465","466","495","496","497","467","468","498","168","169","347","374","499","501","170","502","171","172","503","173","387","335","350","174","504","175","505","176","409","177","506","179","379","507","410","395","180","181","304","508","182","183","184","185","186","364","187","385","188","189","190","191","192","193","194","411","469","509","470","510","511","471","472","558","473","512","474","513","305","234","195","514","196","197","198","200","199","202","201","203","515","204","516","205","206","207","208","360","209","412","388","413","112","517","475","210","476","518","414","211","519","520","212","521","213","307","214","522","523","215","524","216","217","218","525","358","219","526","220","415","221","222","527","223","224","528","225","416","529","530","531","237","532","362","226","227","308","533","229","230","231","232","417","534","233","584","235","236","535");
	v_cat.push_back(Categories(names, ids));
	// anime
	names = ("Art Books","Battle Programer","Big O","Bleach","Cardcaptor Sakura","Chobits","DNAngel","Dragon Ball","Dragonball GT","Dragonball Z","Fanware","Flame of Recca","Full Metal Alchemist","Get Backers","Ghost In..Shell","Gundam","Hajime no Ippo","Inu Yasha","Konkiki No..Bell","Last Exile","Manga","Misc - Movies","Naruto","Onegai Twins","OST Orig..Sndtrack","PlanetES","Ranma 1/2","Raw","Ruroni Kenshin","Samurai Champloo","Scrapped Princess","Stellvia of..Universe","Unsorted");
	ids =("326","238","239","585","240","241","242","396","243","244","328","245","246","248","247","309","249","250","251","252","320","579","253","254","319","255","256","322","257","258","259","260","310");
	v_cat.push_back(Categories(names, ids));
	// hentai
	names = ("Hentai - Movies","Hentai Anime","Hentai Games","Hentai Mangas","Unsorted");
	ids =("580","261","262","263","264");
	v_cat.push_back(Categories(names, ids));
	// linux
	names = ("Applications","Games","Multimedia","Plugins","Programming","Security","Tools","Unsorted");
	ids =("265","271","266","270","269","268","267","272");
	v_cat.push_back(Categories(names, ids));
	// mac
	names = ("Applications","Games","Multimedia","Plugins","Programming","Security","Tools","Unsorted");
	ids =("273","279","274","278","277","276","275","280");
	v_cat.push_back(Categories(names, ids));
	// misc
	names = ("Articles","AudioBook","Comedy/Comics","ComicBooks","DVD/CD-Covers","E-Books","Images","Manual\"s","Radio","Religion","RingTones","SongBooks","Unsorted","WebCast");
	ids =("281","418","283","536","331","282","537","284","538","285","539","540","286","541");
	v_cat.push_back(Categories(names, ids));
	// adult
	names = ("Adult E-Books","Adult Games","Adult Images","Asian AV","Hard Movies","Soft Movies","Unsorted");
	ids =("291","290","289","323","288","287","292");
	v_cat.push_back(Categories(names, ids));
	// unsorted
	names = ("Mixed");
	ids =("293");
	v_cat.push_back(Categories(names, ids));
}

TorrentSpyPoster::~TorrentSpyPoster(void)
{
}

string TorrentSpyPoster::GetMainCategory()
{
	return m_main_category;
}

string TorrentSpyPoster::GetSubCategory() 
{
	return m_sub_category;
}

string TorrentSpyPoster::GetBeginData(string filename, string name, string boundry)
{
	string data = "";
	string cr = "\r\n";
	data += GetFormatedPostData(boundry, "ts", "true"); // terms of  use
	data += GetFormatedPostData(boundry, "lngMainCat", GetMainCategory()); 
	data += GetFormatedPostData(boundry, "lngSubCat", "53");
	data += "--" + boundry + cr;
	data += "Content-Disposition: form-data; name=\"DBFile\"; filename=\"";
	data += filename + "\"" + cr;
	data += "Content-Type: application/octet-stream";
	data += cr + cr;
	return data;
}


string TorrentSpyPoster::GetEndData(string filename, string name, string boundry)
{
	string data = "";
	string cr = "\r\n";
	data += GetFormatedPostData(boundry, "TN", name);
	data += GetFormatedPostData(boundry, "form1", "Upload File");

	data += "--" + boundry + "--" + cr;
	return data;
}

string TorrentSpyPoster::GetPostURL()
{
	return "http://www.torrentspy.com/uploadtorrent.asp?mode=upload";
}

string TorrentSpyPoster::GetRefererURL()
{
	return "http://www.torrentspy.com/uploadtorrent.asp";
}