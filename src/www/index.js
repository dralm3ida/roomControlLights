(function()
{
'use strict';
angular.module("app", [])
.filter("join", [function()
{
   return function (value)
   {
      return value.join(",");
   }
}])
.controller("main", ["$scope", "$http", function ($scope, $http)
{
   $scope.sensors = {p:null
      , ultrasound: [3,55,222,111,33,99,250,20]
      , ultrasoundr: [55,100,55,100,55,100,55,100]
      , light: 0
      , pir: 0
   };// endof ::sensors

   $scope.loadSensors = function ()
   {
      return $http.get("/rest/arduino/com4/all")
      .then(function(response)
      {
         if ( (200 == response.status) && response.data )
         {
            $scope.sensors.ultrasound  = response.data.ultrasound;
            $scope.sensors.light = response.data.light;
            $scope.sensors.pir = response.data.pir;
         }
         console.warn("response", response);
      })
   };// endof ::loadSensors

   $scope.commandVoice = function (command)
   {
     console.warn("original command", command);
      // Filter mispellings
      var commandraw = command.replace(/\b.*ight.*\b/g, "lights").replace(/of\b/g, 'off').replace(/ +/g, '');
      var command = "voice:" + commandraw;


      console.warn("commandVoice", command, commandraw);

      return $http.get("/rest/arduino/com4/" + command)
      .then(function(response)
      {
         console.warn("response", response);
      });
   };// endof ::commandVoice


}])
.directive("myRadar", ["$interval", function ($interval)
{
   return{p:null
      , link: function ($scope, $element, $attrs, $controllers)
      {
         var listener = null;
         var angle = 0;

         listener = $interval(function()
         {
            $element[0].setAttribute("transform", "rotate(" + angle + ")");
            angle += 1;
         }, 15);// endof ::$interval

      }// endof ::postLink
   }
}])
.directive("myPoints", ["$interval", function ($interval)
{
   var nsensors = 8;
   var rx = null, ry = null;
   var centerx = 250;
   var centery = 250;

   var calculateReferencePoints = function (distance, points){
      for ( var i = 0, leni = points.length; i < leni; ++i ){
         rx = centerx + (250-distance[i])*Math.cos(i*(Math.PI/4));
         ry = centery - (250-distance[i])*Math.sin(i*(Math.PI/4));
         points[i].setAttribute("cx", rx);
         points[i].setAttribute("cy", ry);
      }
   }

   return{p:null
      , require: ["ngModel"]
      , link: function ($scope, $element, $attrs, $controllers)
      {
         var points = $element.find("circle");
         var ngModel = $controllers[0];
         var data = null;

         (function()
         {
            ngModel.$formatters.push(function($modelValue)
            {
               data = $modelValue;
               console.warn("myPoints: ", data);
               calculateReferencePoints(data, points);
               return $modelValue;
            });
         }());
         console.warn("ngModel", ngModel);
      }// endof ::postLink
   }
}])
.directive("mySpeechRecognition", ["$parse", "$interval", function ($parse, $interval)
{

   return{p:null
      , scope: true
      , template: "<button class='btn btn-primary' data-ng-click='toggle()' data-ng-disabled='!isLoaded()'>{{ data.label }}</button><label>{{ command }}</label><iframe style='display:none' src='/voice/speech' allow='microphone'></iframe>"
      , compile: function (tElement, tAttrs)
      {
         var fnSettings = $parse(tAttrs.mySpeechRecognition);

         return function postLink ($scope, $element, $attrs, $controllers)
         {
            var iFrameWindow = null, iFrameDocument = null;
            var settings = fnSettings($scope.$parent);
            var iframe = $element.find('iframe');
            var iFrameBtnStart = null;
            var iFrameContents = null;
            var listener = null;
            var text = '';

            var startListening = function ()
            {
               listener = $interval(function()
               {
                  var mytext = '';

                  if ( !iFrameContents )
                  {
                     iFrameContents = iFrameDocument.querySelector("div.ql-editor > p");
                  }
                  mytext = iFrameContents.innerHTML;
                  if ( (mytext != text) && (mytext != '<br>') )
                  {
                     $scope.command = angular.lowercase(mytext.replace(text, ''));
                     text = mytext;
                     settings.callback($scope.command);
                  }
                  if ( 'shut down' == $scope.command )
                  {
                     $scope.toggle();
                  }
               }, 500);
            };// endof ::startListening

            iframe.bind('load', function ()
            {
               iFrameDocument = iframe[0].contentDocument;
               iFrameWindow = iframe[0].contentWindow;
               iFrameBtnStart = iFrameDocument.querySelector(".btn-mic.btn.btn--primary-1");
               $scope.$apply();

               //console.warn("carregou", iframe, iFrameBtnStart);

            });// endof ::load

            $scope.data = { label: 'start' };
            $scope.toggle = function ()
            {
               var myBtn = $element.find('button');
               if ( !listener )
               {
                  iFrameWindow.dictation('clear');
                  iFrameBtnStart.click();
                  startListening();
                  $scope.data.label = 'stop';
                  myBtn.removeClass('btn-primary');
                  myBtn.addClass('btn-danger');
               }
               else
               {
                  $scope.data.label = 'start';
                  iFrameContents = null;
                  myBtn.removeClass('btn-danger');
                  myBtn.addClass('btn-primary');
                  $interval.cancel(listener);
                  listener = null;
                  iFrameWindow.dictation('clear');
                  iFrameBtnStart.click();
               }
            };// endof ::toggle

            $scope.isLoaded = function ()
            {
               return !!iFrameWindow;
            };// endof ::isLoaded

         };// endof ::postLink
      }// endof ::compile
   }

}])





}())
