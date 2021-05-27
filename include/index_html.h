const char * indexPage = R"=====(
<!DOCTYPE html>
<html>
    <head>
        <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.4.1/jquery.min.js"></script>
        <script>
            $(function () {
                setInterval(function () {
                    $.ajax({url: "/getdata" , async: true, 
                    success: function(result){
                        $("#mintemp").html(result['mintemp']);
                        $("#maxtemp").html(result['maxtemp']);
                        $("#interval").html(result['interval']);
                        $("#t1").html(result['t1']);
                        $("#t2").html(result['t2']);
                    }});
                }, 2000);
            });
        </script>
        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">
        <title>SmokeyTwo's Temp Readings</title>
        <style>
            html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}
            body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}
            p {font-size: 24px;color: #444444;margin-bottom: 10px;}
        </style>
    </head>
    <body>
        <header>
            <h1>SmokeyTwo Readout</h1>
            Now twice as sensitive.
        </header>
        <nav>
            <a href="/">home</a>
            <a href="/reset">reboot device</a>
        </nav>
        <section>
            <h2>Basic Set Values</h2>
            <div>Max cutoff <span id="maxtemp"></span>&deg;F</div>
            <div>Min cutoff <span id="mintemp"></span>&deg;F</div>
            <div>check interval <span id="interval"></span>s</div>
        </section>
        <aside>
            <h2> Current State</h2>
            <div>Temperature 1: <span id="t1"></span> &deg;F</div>
            <div>Temperature 2: <span id="t2"></span> &deg;F</div>
        </aside>
    </body>
</html>


)=====";
