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
.controller("main", ["$scope", "$http", "$timeout", function ($scope, $http, $timeout)
{
   $scope.sensors = {p:null
      , ultrasound: [0,0,0,0,0,0,0,0]
      , ultrasoundr: [0,0,0,0,0,0,0,0]
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
         //console.warn("response", response);
         $timeout($scope.loadSensors, 2000);
      });
   };// endof ::loadSensors
   $scope.loadSensors();

   $scope.makeRandom = function ()
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
         //console.warn("response", response);
         $timeout($scope.loadSensors, 2000);
      });
   };// endof ::makeRandom

   $scope.commandVoice = function (command)
   {
      console.warn("original command", command);
      // Filter mispellings
      var commandraw = command
         .replace(/[^ ]*ight[^ ]*/ig, "lights")
         .replace(/of\b/ig, 'off')
         .replace(/what's/ig, 'lights')
         .replace(/\bone\b/ig, '1')
         .replace(/\bgroupon\b/gi, 'group1')
         .replace(/\btwo\b/ig, '2')
         .replace(/ +/ig, '');

      if ( !/^charl.*/gi.test(commandraw) ){ return; }
      console.warn("after replace", commandraw);
      if ( 0 ){}
      else if ( /.*on.*light.*group.*1/ig.test(commandraw) ){ commandraw = "lightsongroup1"; }
      else if ( /.*light.*on.*group.*1/ig.test(commandraw) ){ commandraw = "lightsongroup1"; }
      else if ( /.*of.*light.*group.*1/ig.test(commandraw) ){ commandraw = "lightsoffgroup1"; }
      else if ( /.*light.*of.*group.*1/ig.test(commandraw) ){ commandraw = "lightsoffgroup1"; }
      else if ( /.*on.*light.*group.*2/ig.test(commandraw) ){ commandraw = "lightsongroup2"; }
      else if ( /.*light.*on.*group.*2/ig.test(commandraw) ){ commandraw = "lightsongroup2"; }
      else if ( /.*of.*light.*group.*2/ig.test(commandraw) ){ commandraw = "lightsoffgroup2"; }
      else if ( /.*light.*of.*group.*2/ig.test(commandraw) ){ commandraw = "lightsoffgroup2"; }
      else if ( /.*on.*light.*/ig.test(commandraw) ){ commandraw = "lightson"; }
      else if ( /.*light.*on.*/ig.test(commandraw) ){ commandraw = "lightson"; }
      else if ( /.*of.*light.*/ig.test(commandraw) ){ commandraw = "lightsoff"; }
      else if ( /.*light.*of.*/ig.test(commandraw) ){ commandraw = "lightsoff"; }
      else if ( /.*wake.*/ig.test(commandraw) ){ commandraw = "wakeup"; }
      else if ( /.*shutdown.*/ig.test(commandraw) ){ commandraw = "shutdown"; }
      var command = "voice:" + commandraw;

      console.warn("commandVoice", command, commandraw);

      return $http.get("/rest/arduino/com4/" + angular.lowercase(command))
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
.directive("mySpeechRecognition", ["$parse", "$interval", "$timeout", function ($parse, $interval, $timeout)
{

   return{p:null
      , scope: true
      , template: "<button class='btn btn-primary' data-ng-click='toggle()' data-ng-disabled='!isLoaded()'>{{ data.label }}</button>"+
                  //"<button class='btn btn-default' data-ng-click='clear()'>clear</button>"+
                  "<label>{{ command }}</label><iframe style='display:none' src='/voice/speech' allow='microphone'></iframe>"
      , compile: function (tElement, tAttrs)
      {
         var fnSettings = $parse(tAttrs.mySpeechRecognition);

         return function postLink ($scope, $element, $attrs, $controllers)
         {
            var iFrameWindow = null, iFrameDocument = null, iFrameLog = null;
            var settings = fnSettings($scope.$parent);
            var iframe = $element.find('iframe');
            var lastProcessedText = '', lastDetection = '';
            var iFrameBtnStart = null;
            var iFrameContents = null;
            var listener = null;
            var timeout = null;

            var startListening = function ()
            {
               listener = $interval(function()
               {
                  var mytext = '';

                  if ( !iFrameContents )
                  {
                     iFrameContents = iFrameDocument.querySelector("div.ql-editor > p");
                     iFrameLog = iFrameDocument.querySelector("p.log > span");
                     console.warn("load", iFrameLog);
                  }
                  if ( !!iFrameLog && timeout && (lastDetection != iFrameLog.innerHTML) )
                  {
                     console.warn("cancel timeout 1", lastDetection, iFrameLog.innerHTML);
                     lastDetection = iFrameLog.innerHTML;
                     $timeout.cancel(timeout);
                  }
                  if ( !!iFrameContents )
                  {
                     mytext = iFrameContents.innerHTML;
                     mytext = ('<br>' == mytext)?(''):(mytext);
                     if ( lastProcessedText != mytext )
                     {
                        lastProcessedText = mytext;
                        iFrameWindow.dictation('clear');
                        reloadMicro();
                        if ( '' != mytext )
                        {
                           $scope.command = mytext;
                        }
                        settings.callback($scope.command);
                        //console.warn("mytext", mytext, lastProcessedText);
                        if ( timeout )
                        {
                           console.warn("cancel timeout 2", lastDetection, iFrameLog.innerHTML);
                           $timeout.cancel(timeout);
                        }
                        timeout = $timeout(reloadMicro, 5000);
                     }
                     /*
                     if ( (mytext != text) && (mytext != '<br>') )
                     {
                        $scope.command = angular.lowercase(mytext.replace(text, ''));
                        text = mytext;
                        settings.callback($scope.command);
                     } 
                     */ 
                  }
               }, 100);
            };// endof ::startListening

            var reloadMicro = function ()
            {
               if ( listener )
               {
                  //iFrameWindow.dictation('clear');
                  //text = '';
                  iFrameBtnStart.click();
                  $timeout(function(){
                     iFrameBtnStart.click();
                     //$timeout(reloadMicro, 30000);
                     iFrameContents = iFrameDocument.querySelector("div.ql-editor > p");
                     iFrameLog = iFrameDocument.querySelector("p.log > span");
                  }, 500);
               }
            };// endof ::reloadMicro

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
                  $scope.data.label = 'stop';
                  myBtn.eq(0).removeClass('btn-primary');
                  myBtn.eq(0).addClass('btn-danger');
                  startListening();
                  //$timeout(reloadMicro, 30000);
               }
               else
               {
                  $scope.data.label = 'start';
                  iFrameContents = null;
                  myBtn.eq(0).removeClass('btn-danger');
                  myBtn.eq(0).addClass('btn-primary');
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


            /*
            $scope.clear = function ()
            {
               iFrameWindow.dictation('clear');
            };// endof ::clear
            */

         };// endof ::postLink
      }// endof ::compile
   }

}])





}())
