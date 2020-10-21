<?php
$type = (!empty($_GET['type'])) ? $_GET['type'] : 'bad';

if ($type !== 'forbes' && $type !== 'newyorker' && $type !== 'nytimes') {
    echo "type not supported";
    $output = shell_exec("/home4/nerillos/public_html/log_error.sh $type");
    return;
}

include('getHub.php');

$output = file_get_contents($type . '.json');
$outputArray = json_decode($output, true);
foreach($outputArray as $i => $item) {
	$html = $outputArray[$i]['html'];
	if($html === FALSE) $html='';
        $html = base64_decode($html);

	$pattern = '/<b>|<strong>/s';
	$html = preg_replace($pattern, '&lt;b&gt;', $html);

	$pattern = '/<br><br>/s';
	$html = preg_replace($pattern, '&#10;&#10;', $html);

	$pattern = '/<\/b>|<\/strong>/s';
	$html = preg_replace($pattern, '&lt;/b&gt;', $html);

	$pattern = '/&#xA0;/s';
	$html = preg_replace($pattern, ' ', $html);

//	$pattern = '/&amp;quot;/s';
//	$html = preg_replace($pattern, '&quot;', $html);

//	$pattern = '/&amp;eacute;/s';
//	$html = preg_replace($pattern, '&eacute;', $html);

//	$pattern = '/&amp;egrave;/s';
//	$html = preg_replace($pattern, '&egrave;', $html);

//	$pattern = '/&amp;Ouml;/s';
//	$html = preg_replace($pattern, '&Ouml;', $html);

//	$pattern = '/(&amp;)(lsqb;|rsqb;|quot;|eacute;|egrave;|Ouml;)/s';
//	$html = preg_replace($pattern, '&$2', $html);

	$pattern = '/(&amp;)([a-zA-z0-9]{3,6};)/s';
	$html = preg_replace($pattern, '&$2', $html); // replaces &amp;egrave; with &egrave;

	$html = filter_var($html, FILTER_SANITIZE_STRING);//removes all xml characters

	$outputArray[$i]['html'] = $html;

	$title = filter_var($outputArray[$i]['title'], FILTER_SANITIZE_STRING);
	$outputArray[$i]['title'] = $title;

	$url = filter_var($outputArray[$i]['url'], FILTER_SANITIZE_STRING);
	$outputArray[$i]['url'] = $url;
}
echo json_encode($outputArray); //convert the array to JSON string

?>
