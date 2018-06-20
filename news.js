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

    function setTimeOnScreen(){
	var date = new Date();
	$("#timer").text(date.toDateString() + ' ' + date.toLocaleTimeString());
    }

    function htmlDecode(input){
	var e = document.createElement('div');
	e.innerHTML = input;
      	// handle case of empty input
	return e.childNodes.length === 0 ? "" : e.childNodes[0].nodeValue;
    }

    function getHtml(html) {
        //return html.replace(/^[^(A-Z]+(.*)/g, "$1").  // this was filtering out \u201C left quotes at beginning
	return html.replace(/[\n]&#10;[\n]/g, "\n\n").
	replace(/&amp;nbsp;/g, " ");
    }

    function tumia(evt){
	window.open(evt.target.nextSibling, '_blank');
    }

    function cuando(evt){
	if($(evt.target).hasClass("clickable")){
		evt.target.style.height = evt.target.clientHeight*2 + "px";
		$(evt.target).removeClass("clickable").addClass("doubled");
		$(evt.target).css("cursor","");
	}
    }

    function mary(evt){
    	var link = savetds[0].find('a:first')[0];
    	window.open(link, '_blank');
    }


    function openNews(evt, newsType, newsName) {
        $(".tablinks").removeClass("active");

	var urll = evt.target ? ("getNews.php?type=" + newsType) : "getBloom.php";
        $.getJSON(urll, function (news) {
                $("#newsList").empty();
                $("#newsTemplate").tmpl(news).appendTo("#newsList");
                $(".links").each(function(){
                        var dd = htmlDecode($(this)[0].childNodes[0].nodeValue);
                        $(this)[0].childNodes[0].nodeValue = dd;
                });
                setTimeOnScreen();
                $(".htext").each(function(){
                        var dina = $(this).html().replace(/&lt;\/b&gt;/g,'</b>').replace(/&lt;b&gt;/g,'<b>');
                        $(this).html('');
                        var pre = $("<pre> " + dina + " </pre>");
                        $(this).append(pre);
                        if(!$(this).OverFlowed()){
                                $(this).parent().find("a").remove(); // Remove the ...more link
                        }
			if(newsType > 8 && newsType < 14) //spanish
                        	document.getElementById("title").innerHTML = "<br>Noticias  de  " + newsName;
                        else
				document.getElementById("title").innerHTML = newsName + " News";
			$(evt.target).addClass("active");
                });
        });
    }

    function hrefClick(event){
	event.preventDefault();
	var elem = $(event.target).parent().next().next();
	var english = false;
	if($(event.target).text() == "less..." || $(event.target).text() == "more..."){
		english = true;
	}
	if($(event.target).text() == "mas..." || $(event.target).text() == "more..."){
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
		savetds.push($(event.target).parent().parent().next().clone()); //remove the img and date

		var immg = $(event.target).parent().parent().next().find('img:first').clone();

		$(event.target).parent().parent().next().remove();
		savetds.push($(event.target).parent().parent().next().clone());

		$(event.target).parent().parent().next().remove();

		var ttd = $(event.target).parent().parent();
		ttd.attr("colspan", "3"); //have td occupy all 3 columns
		ttd.find('img:first').addClass("clickable").css("cursor","pointer");
		var pre =  $(event.target).parent().parent().find('pre:first');
	        pre.dblclick(function(event){ //when clicking on the expanded pre text, un-expand it
			event.stopPropagation();
			$(event.target).unbind( "dblclick" );
			$(event.target).parent().parent().find('a:first').click(); // click on less link
        	});

		immg.css("float","right").removeAttr("onclick").attr("onclick","mary(event)");
		immg.insertAfter($(event.target).parent().parent().find('img:first'));
		elem.removeClass("htext");
        	$(event.target).text(english? "less..." : "menos...");
        }else{
		$(event.target).parent().parent().parent().append(savetds[0]);
		$(event.target).parent().parent().parent().append(savetds[1]);
		savetds = [];
		$(event.target).parent().parent().removeAttr("colspan");
		var iimg = $(event.target).parent().parent().find('img:first');
		iimg.removeClass("clickable");
                if(iimg.hasClass("doubled")){
	                iimg.removeClass("doubled");
        	        iimg[0].style.height = iimg[0].clientHeight/2 + "px";
                }
		elem.next().addClass("htext");
        	$(event.target).text(english? "more..." : "mas...");
        	$(event.target).parent().parent().find('img.user-tumb').remove();

        }
		setTimeout(function(){ $(event.target).scrollView();}, 10);
    }

    function getDate(jsonDate, language) { //2016-12-13 19:21:45 
        var tokens = jsonDate.split(" ");
        jsonDate = tokens[0] + "T" + tokens[1] + "Z"; //2016-12-13T19:21:45Z UTC time
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
