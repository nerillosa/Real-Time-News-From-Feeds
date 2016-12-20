<?php

	include('tracker.php');

	$type = (!empty($_GET['type'])) ? $_GET['type'] : 'bad'; 

	if (filter_var($type, FILTER_VALIDATE_INT) === false) {
    		echo("type variable is not an integer");
		$output = shell_exec("/home1/neri/log_error.sh $type");
    		return;	
	}

	$db = new MySqli('localhost', 'xxxxx', 'xxxxx', 'xxxxx');

	$result = $db->query("SELECT MAX(id) FROM news_type");
	$row = $result->fetch_row();
        $maxType = $row[0];

	if($type > $maxType || $type < 1) {
		echo("Non valid type");
		$output = shell_exec("/home1/neri/log_error.sh $type");
		return;
	}


	$news = $db->query("SELECT a.*,b.logo from news a join agency b on a.agency=b.shortname where a.news_type=" . $type . " order by pubdate desc LIMIT 20");
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
		$html = filter_var($html, FILTER_SANITIZE_STRING);
		if($img === 'http://www.foxnews.com/content/dam/fox-news/logo/og-fn-foxnews.jpg') $img = '';
		$news_r[] = array('html' => $html, 'pubdate' => $pubdate, 'url' => $url, 'title' => $title, 'agency' => 
		$agency,'logo' => $logo,'img'=>$img );
	}

	echo json_encode($news_r); //convert the array to JSON string

?>



