<?php

	$type = (!empty($_GET['type'])) ? $_GET['type'] : 'bad'; 

	if (filter_var($type, FILTER_VALIDATE_INT) === false) {
    		echo("type variable is not an integer");
    		return;	
	}

	if($type > 7 || $type < 1) {
		echo("Non valid type");
		return;
	}

	$db = new MySqli('localhost', 'xxxx', 'xxxx', 'xxxx');

	$news = $db->query("SELECT a.*,b.logo from news a join agency b on a.agency=b.shortname where a.news_type=" . $type . " order by pubdate desc LIMIT 20");
	$news_r = array();

	while($row = $news->fetch_array()){
      		//default news data
      		$pubdate = $row['pubdate'];
		$url = filter_var($row['url'], FILTER_SANITIZE_URL);
      		$title = filter_var($row['title'], FILTER_SANITIZE_STRING);
      		$agency = filter_var($row['agency'], FILTER_SANITIZE_STRING);
      		$logo = filter_var($row['logo'], FILTER_SANITIZE_STRING);

		$news_r[] = array('pubdate' => $pubdate, 'url' => $url, 'title' => $title, 'agency' => $agency,'logo' => $logo );
	}

	echo json_encode($news_r); //convert the array to JSON string

?>



