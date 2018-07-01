<?php

//$output = shell_exec("/home1/nerillos/public_html/createBloomberg bloom_urls.txt");
$output = file_get_contents('bloom.json');
$outputArray = json_decode($output, true);

foreach($outputArray as $i => $item) {
	$html = base64_decode($outputArray[$i]['html']);

	if($html === FALSE) $html='';

	$pattern = '/<b>|<strong>/s';
	$html = preg_replace($pattern, '&lt;b&gt;', $html);

	$pattern = '/<br><br>/s';
	$html = preg_replace($pattern, '&#10;&#10;', $html);

	$pattern = '/<\/b>|<\/strong>/s';
	$html = preg_replace($pattern, '&lt;/b&gt;', $html);

	$html = filter_var($html, FILTER_SANITIZE_STRING);//removes all xml characters
	$outputArray[$i]['html'] = $html;

	$title = filter_var($outputArray[$i]['title'], FILTER_SANITIZE_STRING);
	$outputArray[$i]['title'] = $title;

	$url = filter_var($outputArray[$i]['url'], FILTER_SANITIZE_STRING);
	$outputArray[$i]['url'] = $url;
}
echo json_encode($outputArray); //convert the array to JSON string

?>
