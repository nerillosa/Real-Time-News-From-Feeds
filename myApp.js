var styl = '<style id ="styl" type="text/css"> body {background-color: #ff9933; margin: 0;} #MainBody{height: auto;padding: 30px 100px;line-height: 1.5;} #MainBody > p:first-child,p:nth-child(3){text-align: center;} #MainBody > h1:nth-child(2){text-align: center;} #MainBody a:link{background-color: #ff9933;} </style>';

$(document).ready(function(){
	$.ajax({
      		url: "getHub.php?type=Home",
       		cache: false,
       		success: function (data){}
	});
});

function getArticle(typ){
	location.href = "http://nllosa.com/" + typ + ".html";
//     $(".menu>ul>li").removeClass("current-item");
//     $("#linda").addClass("current-item");
//     $.ajax({
//              url: "getHub.php?type=" + typ,
//              cache: false,
//              success: function (data){
//                    $("#MainBody").empty();
//		    $(styl).insertBefore($("#MainBody"));
//		    $("#MainBody").append(data);
//                    $('<input>').attr({ type: 'hidden',value: typ, name: 'app'}).appendTo('form');
//              }
//     });
}

function getAbout(evt){
       $(".menu>ul>li").removeClass("current-item");
       $(evt.target).parent().addClass("current-item");
       $("#styl").remove();
       $("#MainBody").empty().append("<iframe src='about2.html' frameborder='0' style='overflow:hidden;height:100%;width:100%' height='100%' width='100%'></iframe>");
}

function getBlog(evt){
       $(".menu>ul>li").removeClass("current-item");
       $(evt.target).parent().addClass("current-item");
       $("#styl").remove();
       $("#MainBody").empty().append("<iframe src='blog.html' frameborder='0' style='overflow:hidden;height:100%;width:100%' height='100%' width='100%'></iframe>");
}
