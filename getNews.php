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

	//This query is a bit long but it gets rid of duplicate images for same news/changed titles
	if($type <= $maxType){
	   $news = $db->query("SELECT a.*,b.logo from news a join agency b on a.agency=b.shortname join (select max(pubdate)" .
	   " as pubdate, max(title) as title,img from news group by img)c on a.pubdate=c.pubdate and a.img=c.img and a.title=c.title" .
	   " where a.news_type=" . $type .	" order by a.pubdate desc LIMIT 20");
	} else{
	//Trump news
           $news = $db->query("select c.*,b.logo from (select max(create_date)as create_date,html,img from news group by" .
           " html,img) a join (select * from news) c on a.create_date=c.create_date and a.img=c.img and a.html=c.html" .
           " join agency b on c.agency=b.shortname where FROM_BASE64(c.html) like '%Trump%'" .
           " and c.news_type<9 order by c.pubdate desc limit 20");
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

/*
	        $pattern = '/<h3>/s';
        	$html = preg_replace($pattern, '&lt;b&gt;', $html);
	        $pattern = '/<strong>/s';
        	$html = preg_replace($pattern, '&lt;b&gt;', $html);

	        $pattern = '/<\/h3>/s';
        	$html = preg_replace($pattern, '&lt;/b&gt;', $html);
	        $pattern = '/<\/strong>/s';
        	$html = preg_replace($pattern, '&lt;/b&gt;', $html);
*/
	        $html = filter_var($html, FILTER_SANITIZE_STRING);


		$news_r[] = array('html' => $html, 'pubdate' => $pubdate, 'url' => $url, 'title' => $title, 'agency' =>
		$agency,'logo' => $logo,'img'=>$img );
	}

	echo json_encode($news_r); //convert the array to JSON string

?>



