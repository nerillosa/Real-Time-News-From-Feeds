<?php

	include('tracker.php');

	$type = (!empty($_GET['type'])) ? $_GET['type'] : 'bad';

	if (filter_var($type, FILTER_VALIDATE_INT) === false) {
    		echo("type variable is not an integer");
		$output = shell_exec("/home1/nerillos/public_html/log_error.sh $type");
    		return;
	}

	$db = new MySqli('localhost', 'nerillos_neri', 'carpa1', 'nerillos_neri');

	$result = $db->query("SELECT MAX(id) FROM news_type");
	$row = $result->fetch_row();
        $maxType = $row[0];

	if($type > ($maxType+1) || $type < 1) {
		echo("Non valid type");
		$output = shell_exec("/home1/nerillos/public_html/log_error.sh $type");
		return;
	}

        if($type <= $maxType){
           $news = $db->query("select c.*,d.logo from news c join agency d on c.agency=d.shortname where" .
           " c.news_type=" . $type . " order by c.pubdate desc limit 20");
        }
//        else{
//	   // Trump news
//           $news = $db->query("select c.*,d.logo from" .
//           " (select a.* from (select * from news) a, (select max(create_date)as create_date,img" .
//           " from news group by img) b where a.create_date=b.create_date and a.img=b.img) c" .
//           " join agency d on c.agency=d.shortname where FROM_BASE64(c.html) like '%Trump%'" .
//           " and c.news_type<9 order by c.pubdate desc limit 20");
//        }

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

	        $pattern = '/<b>|<strong>/s';
		if($agency != "COMERCIO" && $agency != "RPP")
        		$html = preg_replace($pattern, '&lt;b&gt;', $html);

	        $pattern = '/<br><br>/s';
        	$html = preg_replace($pattern, '&#10;&#10;', $html);

	        $pattern = '/<\/b>|<\/strong>/s';
        	if($agency != "COMERCIO" && $agency != "RPP")
        		$html = preg_replace($pattern, '&lt;/b&gt;', $html);

	        $html = filter_var($html, FILTER_SANITIZE_STRING);//removes all xml characters

		// If it's not already UTF-8, convert to it. Mysql saves in iso-8859-1 (Latin 1)
		// Solves the bad chars \u00e2\u0080\u0098 and \u00e2\u0080\u0099 which are converted to
		// \u2018 and \u2019 which are utf-8 left and right double quotes
		if (mb_detect_encoding($title, 'utf-8', true) === false) { //not utf-8
    			$title = mb_convert_encoding($title, 'utf-8', 'iso-8859-1'); //convert from latin1 to utf-8
		}


                //need to utf_encode title as json_encode was bombing with JonBenét Ramsey (accented e)
                //$news_r[] = array('html' => $html, 'pubdate' => $pubdate, 'url' => $url, 'title' => utf8_encode($title),
                //'agency' =>$agency,'logo' => $logo,'img'=>$img );

                $news_r[] = array('html' => $html, 'pubdate' => $pubdate, 'url' => $url, 'title' => $title,
                'agency' =>$agency,'logo' => $logo,'img'=>$img );
	}
	echo json_encode($news_r); //convert the array to JSON string

?>



