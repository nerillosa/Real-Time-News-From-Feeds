<?php

$text = file_get_contents('http://finance.yahoo.com/webservice/v1/symbols/allcurrencies/quote');
$domain = strstr($text, '<field name="name">USD/PEN</field>');
$pattern = '~<field name="price">(.*)</field>~';

if (preg_match($pattern, $domain, $match) ) {
	echo "Tipo de Cambio: $match[0] Nuevos Soles por D&oacute;lar USA";
}else{
	echo "";
}
?>





