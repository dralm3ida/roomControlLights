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
      , ultrasound: [0,0,0,0,0,0,0,0]
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
      var command = "voice:" + command.replace(/ +/g, '').replace(/of\b/g, 'off');

      return $http.get("/rest/arduino/com4/" + command)
      .then(function(response)
      {
         console.warn("response", response);
      });
   };// endof ::commandVoice


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
