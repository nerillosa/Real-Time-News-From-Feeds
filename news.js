    var savetds = [];

    $(document).ajaxStart(function(){$('.loadingDiv').show();});
    $(document).ajaxStop(function(){$('.loadingDiv').hide();});
    $(document).ready(function(){
    	$(".tablinks").eq(5).click();

	$("div.box-table").eq(1).click(function(e){
		if (e.target !== this)//only click on parent
			return;
	    	$('html, body').animate({scrollTop: $("div.box-table").eq(0).offset().top}, 250);
	});
    });

    $.fn.OverFlowed = function() {
    	var _elm = $(this)[0];
    	var _hasScrollBar = false;
    	if ((_elm.clientHeight < _elm.scrollHeight) || (_elm.clientWidth < _elm.scrollWidth)) {
        	_hasScrollBar = true;
    	}
    	return _hasScrollBar;
    };

    $.fn.scrollView = function () {
  	return this.each(function () {
    	$('html, body').animate({
      	scrollTop: $(this).offset().top
    	}, 250);
        });
    };

    function htmlDecode(input){
	var e = document.createElement('div');
	e.innerHTML = input;
      	// handle case of empty input
	return e.childNodes.length === 0 ? "" : e.childNodes[0].nodeValue;
    }

    function getHtml(html) {
	return html.replace(/^[^(A-Z]+(.*)/g, "$1").
	replace(/[\n]&#10;[\n]/g, "\n\n");
    }

    function tumia(evt){
	window.open(evt.target.nextSibling, '_blank');
    }

    function mary(evt){
    	var link = savetds[0].find('a:first')[0];
    	window.open(link, '_blank');
    }

    function openNews(evt, newsType, newsName) {
	$(".tablinks").removeClass("active");
	$.getJSON("getNews.php?type=" + newsType, function (news) {
		$("#newsList").empty();
		$("#newsTemplate").tmpl(news).appendTo("#newsList");
		$(".links").each(function(){
			var dd = htmlDecode($(this)[0].childNodes[0].nodeValue);
			$(this)[0].childNodes[0].nodeValue = dd;
		});
		$(".htext").each(function(){
			var dina = $(this).html().replace(/&lt;\/b&gt;/g,'</b>').replace(/&lt;b&gt;/g,'<b>');
			$(this).html('');
			var pre = $("<pre> " + dina + " </pre>");
			$(this).append(pre);
			if(!$(this).OverFlowed()){
				$(this).parent().find("a").remove(); // Remove the ...more link
			}
			document.getElementById("title").innerHTML = newsName + " News";
			$(evt.target).addClass("active");
		});
	});
    }

    function getDate(jsonDate) { //2016-12-13 19:21:45 mysql DateTime
        var tokens = jsonDate.split(" ");
        jsonDate = tokens[0] + "T" + tokens[1] + "Z"; //2016-12-13T19:21:45Z UTC time
	    var jdt = new Date(jsonDate);
  	    var rvalue = Math.ceil((Date.now() - jdt.getTime())/60000);
  	    if (rvalue < 60 && rvalue > 0)
  	        return "" + rvalue + (rvalue === 1 ? " minute" : " minutes") + " ago";
  	    else {
  	        return jdt.toLocaleString();
  	    }
    }

    function hrefClick(event){
	event.preventDefault();
	var elem = $(event.target).parent().next().next();
	if($(event.target).text() == "more..."){
		if(savetds.length >0){
			var tdd = $("table tbody").find("td[colspan='3']");
			tdd.find('img:last').remove();
                        tdd.find('pre:first').unbind( "click" );
		        tdd.parent().append(savetds[0]);
		        tdd.parent().append(savetds[1]);
		        savetds = [];
			tdd.removeAttr("colspan");
			$(":last-child", tdd).addClass("htext");
			tdd.find("a").text("more...");
		}
		savetds.push($(event.target).parent().parent().next().clone()); //remove the img and date

		var immg = $(event.target).parent().parent().next().find('img:first').clone();

		$(event.target).parent().parent().next().remove();
		savetds.push($(event.target).parent().parent().next().clone());

		$(event.target).parent().parent().next().remove();
		$(event.target).parent().parent().attr("colspan", "3"); //have td ocuppy all 3 columns
		var pre =  $(event.target).parent().parent().find('pre:first');
	        pre.click(function(event){ //when clicking on the expanded pre text, un-expand it
			event.stopPropagation();
			$(event.target).unbind( "click" );
			$(event.target).parent().parent().find('a:first').click(); // click on less link
        	});

		immg.css("float","right").removeAttr("onclick").attr("onclick","mary(event)");
		immg.insertAfter($(event.target).parent().parent().find('img:first'));
		elem.removeClass("htext");
        	$(event.target).text("less...");
        }else{
		$(event.target).parent().parent().parent().append(savetds[0]);
		$(event.target).parent().parent().parent().append(savetds[1]);
		savetds = [];
		$(event.target).parent().parent().removeAttr("colspan");
		elem.next().addClass("htext");
        	$(event.target).text("more...");
        	$(event.target).parent().parent().find('img.user-tumb').remove();

        }
		setTimeout(function(){ $(event.target).scrollView();}, 10);
    }
