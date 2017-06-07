var app = angular.module("myApp", ["ngRoute"]);
app.config(function ($routeProvider) {
    $routeProvider
        .when("/", {
            templateUrl: "home.html",
            controller: "TemplateCtrl"
        })
        .when("/contact", {
            template: "<h1 style='padding-top:50px;text-align:center;'>https://facebook.com/neri.llosa <br><br> Email: nerillosa@cox.net <br><br>Twitter: https://twitter.com/nllosa</h1>",
            controller: "TemplateCtrl"
        })
        .when("/about", {
            template: "<iframe src='about2.html' frameborder='0' style='overflow:hidden;height:100%;width:100%' height='100%' width='100%'></iframe>",
            controller: "TemplateCtrl"
        })
        .when("/selected/:inndex", {
            templateUrl: "selected.tpl.html",
            controller: "SelectedCtrl"
        })
        .when("/usa_news/:inndex", {
            templateUrl: "listbox.tpl.html",
            controller: "NewsCtrl"
        })
        .otherwise({
            redirectTo: "home.html",
            controller: "TemplateCtrl"
        });
});

app.directive('newsDiv', function () {
    var directive = {};

    directive.restrict = 'A'; /* restrict this directive to attributes */

    directive.compile = function (element, attributes) { // compile gets executed once per each occurence of directive element
        element.css("padding-left", "5px").css("overflow", "hidden").css("height", "110px").css("line-height", "14px");
        var linkFunction = function ($scope, element, attributes) {
            element.html($scope.news()[attributes.newsDiv].html);
            $scope.htmlChanged(attributes.newsDiv, element); //check to see if overflowed so to add the more... link
        }
        return linkFunction; //called every time the element is to be bound to data in the $scope object
    }

    return directive;
});

app.directive('selectedDiv', function () {
    return function ($scope, element, attributes) {
        element.css("margin", "0px 10px").css("height", "190px");
        var htm = $scope.selectedNews.html;
        htm = htm.replace(/^[^(A-Z]+(.*)/g, "$1").replace(/[\n]&#10;[\n]/g, "\n\n");
        //htm = htm.replace(/&lt;\/b&gt;/g, '</b>').replace(/&lt;b&gt;/g, '<br><b>');
        element.append('<pre>' + htm + '</pre>');
    };
});

app.service('NewsUtil', function ($http) {
    var self = this;
    self.news = {};
    self.type = 1; // Default Home

    self.getNews = function () {
        return self.news;
    };

    self.setNews = function (x) {
        var isPeru = x > 8;
        self.type = isPeru ? 3 : 2;
        $http.get("../getNews.php?type=" + x)
            .then(function (response) {
                self.news = response.data.slice(0, 10);
            });
    };

    self.getType = function () {
        return self.type;
    };

    self.setType = function (t) {
        self.type = t;
    };

});

app.controller('MenuCtrl', function ($scope, $route, $routeParams, $location, $log, NewsUtil) {
    $scope.active = function () {
        return NewsUtil.getType();
    };

    $scope.refreshNews = function (path, event) {
        if ($location.path() === path) { //previous url was the same
            $route.reload();
        }
    };
});

app.controller('TemplateCtrl', function ($scope, $route, $routeParams, $location, $log, NewsUtil) {
    if ($location.path() == "/about") {
        NewsUtil.setType(4);
    } else if ($location.path() == "/contact") {
        NewsUtil.setType(5);
    } else {
        NewsUtil.setType(1);
    }
});

app.controller('SelectedCtrl', function ($scope, $http, $route, $routeParams, $location, NewsUtil) {

    window.scrollTo(0, 200); // position to top of the page

    //    $scope.news = function () {
    //        return NewsUtil.getNews();
    //    };

    $scope.selectedNews = NewsUtil.getNews()[parseInt($routeParams.inndex)];

    $scope.goBackText = NewsUtil.getType() == 3 ? "Regresar" : "Go Back";

    $scope.goBack = function () {
        if (NewsUtil.getType() == 1)
            window.location.assign("http://nllosa.com");
        else
            window.history.back();
    };

    //    $scope.inndex = parseInt($routeParams.inndex);

    $scope.unescapeSpecial = function (data) {
        return $('<div/>').html(data).text();
    };

});

app.controller('NewsCtrl', function ($scope, $http, $route, $routeParams, $location, NewsUtil) {

    var headlines = {
        "1": "Politics News",
        "2": "Science News",
        "3": "World News",
        "4": "Sports News",
        "5": "Entertainment News",
        "6": "Health News",
        "7": "USA News",
        "8": "Business News",
        "9": "Noticias de Actualidad",
        "10": "Noticias Deportivas",
        "11": "Noticias Politicas",
        "12": "Noticias de Entretenimiento",
        "13": "Noticias Economicas"
    };

    NewsUtil.setNews(parseInt($routeParams.inndex));
    $scope.headline = headlines[$routeParams.inndex];
    $scope.newsType = parseInt($routeParams.inndex) > 8 ? "mas" : "more";
    $scope.viewArticle = parseInt($routeParams.inndex) > 8 ? "Ver Articulo original" : "View full article";

    $scope.getDate = function (jsonDate) {
        return getDate(jsonDate);
    };

    $scope.htmlChanged = function (index, element) {
        if (isOverflowed(element)) {
            NewsUtil.getNews()[index].show = true; // show the more... link
        }
    };

    $scope.unescapeSpecial = function (data) {
        return $('<div/>').html(data).text();
    };

    $scope.news = function () {
        return NewsUtil.getNews();
    };

    function isOverflowed(elem) {
        if ((elem[0].clientHeight < elem[0].scrollHeight) || (elem[0].clientWidth < elem[0].scrollWidth)) {
            return true;
        }
        return false;
    }

    function getDate(jsonDate) { //2016-12-13 19:21:45 mysql DateTime
        var tokens = jsonDate.split(" ");
        jsonDate = tokens[0] + "T" + tokens[1] + "Z"; //2016-12-13T19:21:45Z UTC time
        var jdt = new Date(jsonDate);
        var rvalue = Math.ceil((Date.now() - jdt.getTime()) / 60000);
        if (rvalue < 60 && rvalue > 0)
            if (parseInt($routeParams.inndex) <= 8)
                return "" + rvalue + (rvalue === 1 ? " minute" : " minutes") + " ago";
            else
                return "Hace " + rvalue + (rvalue === 1 ? " minuto" : " minutos");
        else {
            return jdt.toLocaleString();
        }
    }

});
