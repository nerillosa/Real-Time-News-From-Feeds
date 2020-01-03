    var savetds = [];

    $(document).ajaxStart(function(){$('.loadingDiv').show();});
    $(document).ajaxStop(function(){$('.loadingDiv').hide();});

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

    function setTimeOnScreen(isSpanish){
	var date = new Date();
	if(!isSpanish)
		$("#timer").text(date.toDateString() + ' ' + date.toLocaleTimeString());
	else{
		var options = { weekday: 'long', year: 'numeric', month: 'long', day: 'numeric' };
		$("#timer").text(date.toLocaleDateString('es-ES', options));
	}
    }

    function htmlDecode(input){
	var e = document.createElement('div');
	e.innerHTML = input;
      	// handle case of empty input
	return e.childNodes.length === 0 ? "" : e.childNodes[0].nodeValue;
    }

    function getHtml(html) {
	return html.replace(/^[ \t\n\u00a0]*(&#10;)*[\n]*/g,"").
	replace(/[\n]&#10;[\n]/g, "\n\n").
	replace(/[ ]+\n[ ]*,/g, ",").
	replace(/\n[ ]+/g, "").
	//replace(/&amp;amp;/g, "&amp;").
	//replace(/&amp;frac12;/g, "&frac12;").
	replace(/&amp;nbsp;/g, " ");
    }

    function tumia(evt){
	window.open(evt.target.nextSibling, '_blank');
    }

    function cuando(evt){
	if($(evt.target).hasClass("clickable")){
		evt.target.style.height = evt.target.clientHeight*2 + "px";
		evt.target.style.paddingBottom = "5px";
		$(evt.target).removeClass("clickable").addClass("doubled");
		$(evt.target).css("cursor","");
	}
    }

    function mary(evt){
    	var link = savetds[0].find('a:first')[0];
    	window.open(link, '_blank');
    }

    function compare(a,b) {
      if (a.pubdate > b.pubdate)
        return -1;
      if (a.pubdate < b.pubdate)
        return 1;
      return 0;
    }

    function openNews(evt, newsType, newsName) {
        $(".tablinks").removeClass("active");

	var url3 = "getNews.php?type=" + newsType;

	if(typeof evt.target === 'string'){ //news sites
		$(".tablinks:last").addClass("active");
		var newsSite = evt.target.toLowerCase();
		if(newsSite == "politico")
			url3 = "getPolitico.php";
		else if(newsSite == "bloomberg")
			url3 = "getBloom.php";
		else if(newsSite == "newyorker"){
				url3 = "getPowershell.php?type=newyorker";
				newsName = "The New Yorker";
			}
		else if(newsSite == "forbes"){
				url3 = "getPowershell.php?type=forbes";
				newsName = "Forbes";
			}
		else if(newsSite == "nytimes"){
				url3 = "getPowershell.php?type=nytimes";
				newsName = "NY Times";
			}
		else if(newsSite == "huffingtonpost"){
				url3 = "getHuff.php";
				newsName = "Huffington Post";
			}
		else if(newsSite == "washingtonpost"){
				url3 = "getWashingtonPost.php";
				newsName = "Washington Post";
			}
		else if(newsSite == "time"){
				url3 = "getTime.php";
				newsName = "Time Magazine";
			}
	}

        $.getJSON(url3, function (news) {
		if(typeof evt.target === 'string'){ //news sites
			news.sort(compare);
			var i = news.length;
			var oldImg = "";
			while (i--) {
				if (!news[i].title || !news[i].html || news[i].html.length < 200) {
					news.splice(i, 1);
    				}else {
    					if(oldImg == news[i].img){
    						news.splice(i, 1);
    					}else{
    				    		oldImg = news[i].img;
    					}
    				}
			}
		}else{
			$(evt.target).addClass("active");
		}

                $("#newsList").empty();
                $("#newsTemplate").tmpl(news).appendTo("#newsList");
                $(".links").each(function(){
                        var dd = htmlDecode($(this)[0].childNodes[0].nodeValue);
                        $(this)[0].childNodes[0].nodeValue = dd;
                });
                setTimeOnScreen(newsType > 8 && newsType < 14);
                $(".htext").each(function(){
                        var dina = $(this).html().replace(/&lt;\/b&gt;/g,'</b>').replace(/&lt;b&gt;/g,'<b>');
                        $(this).html('');
                        var pre = $("<pre> " + dina + " </pre>");
                        $(this).append(pre);
                        if(!$(this).OverFlowed()){
                                $(this).parent().find("a").remove(); // Remove the ...more link
                        }
                });
		if(newsType > 8 && newsType < 14) //spanish
			document.getElementById("title").innerHTML = "<br>Noticias  de  " + newsName;
		else
			document.getElementById("title").innerHTML = newsName + " News";
        });
    }

    function hrefClick(event){
	event.preventDefault();
	var href = $(event.target);
	var parentTd = href.parent().parent();
	var elem = href.parent().siblings(':last');
	var english = false;
	if(href.text() == "less..." || href.text() == "more..."){
		english = true;
	}
	if(href.text() == "mas..." || href.text() == "more..."){
		if(savetds.length >0){
			var tdd = $("table tbody").find("td[colspan='3']");
			tdd.find('img:last').remove();
                        tdd.find('pre:first').unbind( "dblclick" );
		        tdd.parent().append(savetds[0]);
		        tdd.parent().append(savetds[1]);
		        savetds = [];
			tdd.removeAttr("colspan");
			var immg = tdd.find('img:first');
			immg.removeClass("clickable").css("cursor","");
			if(immg.hasClass("doubled")){
				immg.removeClass("doubled");
				immg[0].style.height = immg[0].clientHeight/2 + "px";
			}
			$(":last-child", tdd).addClass("htext");
			tdd.find("a").text(english? "more..." : "mas...");
		}
		savetds.push(parentTd.next().clone()); //remove the img and date

		var immg = parentTd.next().find('img:first').clone();

		parentTd.next().remove();
		savetds.push(parentTd.next().clone());

		parentTd.next().remove();
		parentTd.attr("colspan", "3"); //have td occupy all 3 columns
		parentTd.find('img:first').addClass("clickable").css("cursor","pointer");

		var pre =  parentTd.find('pre:first');
	        pre.dblclick(function(event){ //when clicking on the expanded pre text, un-expand it
			event.stopPropagation();
			$(event.target).unbind( "dblclick" );
			$(event.target).parent().parent().find('a:first').click(); // click on less link
        	});

		immg.css("float","right").removeAttr("onclick").attr("onclick","mary(event)");
		immg.insertAfter(parentTd.find('img:first'));
		elem.removeClass("htext");
        	href.text(english? "less..." : "menos...");
        }else{
		parentTd.parent().append(savetds[0]);
		parentTd.parent().append(savetds[1]);
		savetds = [];
		parentTd.removeAttr("colspan");
		var iimg = parentTd.find('img:first');
		iimg.removeClass("clickable");
                if(iimg.hasClass("doubled")){
	                iimg.removeClass("doubled");
        	        iimg[0].style.height = iimg[0].clientHeight/2 + "px";
                }
		elem.addClass("htext");
        	href.text(english? "more..." : "mas...");
        	parentTd.find('img.user-tumb').remove();

        }
		setTimeout(function(){ href.scrollView();}, 10);
    }

    function getDate(jsonDate, language, offset) { //2016-12-13 19:21:45
	jsonDate = jsonDate.replace(/T/g, " ");
        var tokens = jsonDate.split(" ");

        if(!offset){
        	offset = "Z";
        }else if(offset.slice(0,-4) === "HUFF" || offset.slice(0,-4) === "POLITICO"){
		offset = "-04:00";
	}
	else{
	        offset = "Z";
	}

        jsonDate = tokens[0] + "T" + tokens[1] + offset;
        //2016-12-13T19:21:45Z (UTC time) or 2016-12-13T19:21:45-04:00 (Huffington)
        var jdt = new Date(jsonDate);
        var rvalue = Math.ceil((Date.now() - jdt.getTime())/60000);
        if (rvalue < 60 && rvalue > 0){
		if(language === 'English')
               		return "" + rvalue + (rvalue === 1 ? " minute" : " minutes") + " ago";
		else
			return "hace " + rvalue + (rvalue === 1 ? " minuto" : " minutos");
	}
	else {
		return jdt.toLocaleString();
	}
    }
