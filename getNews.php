<?php

	include('tracker.php');

	$type = (!empty($_GET['type'])) ? $_GET['type'] : 'bad';

	if (filter_var($type, FILTER_VALIDATE_INT) === false) {
    		echo("type variable is not an integer");
		$output = shell_exec("/************/log_error.sh $type");
    		return;
	}

	$db = new MySqli('localhost', '***********', '********', '************');

	$result = $db->query("SELECT MAX(id) FROM news_type");
	$row = $result->fetch_row();
        $maxType = $row[0];

	if($type > ($maxType+1) || $type < 1) {
		echo("Non valid type");
		$output = shell_exec("/home******************** $type");
		return;
	}

        if($type <= $maxType){
        	if($type == 13){ // first 10 of each agency
        		$queryString =
        		"(select * from news c join agency d on c.agency=d.shortname where c.news_type=13 and " .
        		"d.shortname='REPUBLICA' order by pubdate desc limit 10)" .
        		" UNION " .
        		"(select * from news c join agency d on c.agency=d.shortname where c.news_type=13 and " .
        		"d.shortname='PERU' order by pubdate desc limit 10)";
        		$news = $db->query($queryString);
        	}else{
        		$news = $db->query("select c.*,d.logo from news c join agency d on c.agency=d.shortname where" .
           		" c.news_type=" . $type . " order by c.pubdate desc limit 20");
           	}
        }

	$news_r = array();

	while($row = $news->fetch_array()){
      		//default news data
      		$pubdate = $row['pubdate'];
		$url = filter_var($row['url'], FILTER_SANITIZE_URL);
      		$title = filter_var($row['title'], FILTER_SANITIZE_STRING);
      		$agency = filter_var($row['agency'], FILTER_SANITIZE_STRING);
      		$logo = filter_var($row['logo'], FILTER_SANITIZE_STRING);
      		$img = filter_var($row['img'], FILTER_SANITIZE_URL);
		$html = base64_decode($row['html']);
		if($html === FALSE) $html='';

	        $html = filter_var($html, FILTER_SANITIZE_STRING);//removes all xml characters

		if (mb_detect_encoding($title, 'utf-8', true) === false) { //not utf-8
    			$title = mb_convert_encoding($title, 'utf-8', 'iso-8859-1'); //convert from latin1 to utf-8
		}
		//http://www.conandalton.net/2011/03/convert-your-mysql-database-from-latin.html
		$htitle = bin2hex($title); 
		$htitle = str_replace("c3a2e282ace2809c","e28093",$htitle); // EN Dash
		$htitle = str_replace("c3a2e282ace2809d","e28094",$htitle); // EM Dash
		$htitle = str_replace("c3a2e282acc593","e2809c",$htitle); // left double quotation mark
		$htitle = str_replace("c3a2e282acc29d","e2809d",$htitle); // right double quotation mark
		$htitle = str_replace("c3a2e282accb9c","e28098",$htitle); // left single quote
		$htitle = str_replace("c3a2e282ace284a2","e28099",$htitle); // right single quote

		$htitle = str_replace("c383c2a1","c3a1",$htitle); // a accent
		$htitle = str_replace("c383c2a9","c3a9",$htitle); // e accent
		$htitle = str_replace("c383c2ad","c3ad",$htitle); // i accent
		$htitle = str_replace("c383c2b3","c3b3",$htitle); // o accent
		$htitle = str_replace("c383c2ba","c3ba",$htitle); // u accent

		$htitle = str_ireplace("C383C281","c381",$htitle); // A accent
		$htitle = str_ireplace("C383E280B0","c389",$htitle); // E accent
		$htitle = str_ireplace("C383C28D","c38d",$htitle); // I accent
		$htitle = str_ireplace("C383E2809C","c393",$htitle); // O accent
		$htitle = str_ireplace("C383C5A1","c39a",$htitle); // U accent

		$htitle = str_replace("c383c2b1","c3b1",$htitle); // ñ
		$htitle = str_replace("C383E28098","c391",$htitle); //capital // ñ
		$htitle = str_replace("c382c2bf","c2bf",$htitle); // inverted question mark

		$title = hex2bin($htitle);
                //echo bin2hex($title);
		//echo "\n";
                $news_r[] = array('html' => $html, 'pubdate' => $pubdate, 'url' => $url, 'title' => $title, 'agency' =>$agency,'logo' => $logo,'img'=>$img );
	}


	echo json_encode($news_r); //convert the array to JSON string

?>



