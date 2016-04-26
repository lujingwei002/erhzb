<?php

class HttpClient {

    private static $context = null;         // (resource) Socket stream 
    private $header = null;                 // (array) Request headers
    private $headerList = null;             // (array) User custom request headers
    public $buffer = null;                  // (string) Call buffer
    public $response = null;                // (array) Remote response headers
    public $request = null;                 // (string) Request headers
    private $args = null;                   // (array) User configure
    private $attachRedirect = null;         // (bool) Tigger HTTP redirect 

    public function __construct($args  = null) {

        if(!is_array($args)) $args = array();
        $this->args = $args;
        $charset = isset($this->args['charset']) ?  $this->args['charset'] : 'UTF-8';
        if(!empty($this->args['debugging'])) {

            set_time_limit(0);
            header('Content-Type: text/plain;charset='. $charset);

        }else{

            header('Content-Type: text/html;charset='. $charset);

        }

        if(!isset($this->args['timeout'])) $this->args['timeout'] = 5;
        $this->args['timeout'] = intval($this->args['timeout']);

        if(!empty($this->args['redirect'])) $this->attachRedirect = true;
        $this->headerList = array();

    }

    public static function init(& $instanceof, $args = null) {

        static $instance;
        if(!$instance) $instanceof = new self($args);
        return $instance = $instanceof;

    }

    public function setHeader($name, $value) {

        $this->headerList[$name] = $value;

    }

    private function build($args) {

        list($method, $url, $data, $cookie) = $args;

        $this->buffer = '';
        $this->request = '';
        $this->header = array();
        $this->response = array();
        $userAgent = isset($this->args['userAgent']) ? $this->args['userAgent'] : ( isset($_SERVER['HTTP_USER_AGENT']) ? $_SERVER['HTTP_USER_AGENT'] : __CLASS__ );
        extract($parse = parse_url($url));

        $path = isset($query) ? $path .'?'. $query : ( isset($path) ? $path : '/' );
        $port = isset($port) ? $port : ( $scheme == 'https' ? 443 : 80 );
        $protocol = $scheme == 'https' ? 'ssl://' : 'tcp://';

        self::$context = fsockopen($protocol . $host, $port, $errno, $errstr, $this->args['timeout']);
        if($errno) trigger_error(iconv('GBK//IGNORE', 'UTF-8', $errstr), E_USER_ERROR);

        stream_set_blocking(self::$context, 1);
        stream_set_timeout(self::$context, $this->args['timeout']);

        $query = $data;
        if($data && is_array($data)) {

            $query = array();
            foreach($data as $k => $value)
                array_push($query, $k .'='. $value);
            $query = implode('&', $query);

        }

        array_push($this->header, $method .' '. $path .' HTTP/1.1');
        array_push($this->header, 'Host: '. $host);
        array_push($this->header, 'Accept: */*');
        array_push($this->header, 'Content-type: application/x-www-form-urlencoded');
        array_push($this->header, 'Connection: close');
        array_push($this->header, 'User-Agent: '. $userAgent);

        if($this->headerList)
            foreach($this->headerList as $name => $value)
                array_push($this->header, $name .': '. $value);

        if($cookie) array_push($this->header, 'Cookie: '. $cookie );
        if($data) array_push($this->header, 'Content-Length: '. strlen($query));
        if($data) array_push($this->header, '');
        if($data) array_push($this->header, $query);
        array_push($this->header, "\r\n");

        $this->request = implode("\r\n", $this->header);      
        fputs(self::$context, $this->request);

        $skipped = false;
        $this->HTTP_STATUS_CODE = 0;
        $this->HTTP_TRANSFER_CHUNKED = false;

        while(!feof(self::$context)) {

            if(($line = fgets(self::$context))) {

                if(preg_match('/HTTP\/\d\.\d\s*(\d+)/i', $line, $match))
                    $this->HTTP_STATUS_CODE = (int) array_pop($match);

                if(preg_match('/Location:\s*(.+)\s*?/i', $line, $match)) 
                    ( ($this->HTTP_REDIRECT_URL = trim(array_pop($match))) && $skipped = !$skipped );

                if(preg_match('/Transfer\-Encoding:\s*chunked/i', $line, $match)) 
                    $this->HTTP_TRANSFER_CHUNKED = true;
                if(array_push($this->response, $line) && in_array($line, array("\n", "\r\n"))) break;

            }

        }

        if($this->attachRedirect && $skipped) {

            fclose(self::$context);
            $data ? call_user_func_array(array($this, $method), array($this->HTTP_REDIRECT_URL, $data, $cookie)):
                call_user_func_array(array($this, $method), array($this->HTTP_REDIRECT_URL, $cookie));

        }


        if(!$skipped) {

            if($this->HTTP_STATUS_CODE === 200) {

                $this->buffer = '';
                $chunksize = 0;
                $chunked = '';

                while(!feof(self::$context)) {

                    $line = fgets(self::$context);
                    if($this->HTTP_TRANSFER_CHUNKED) {

                        if(!$chunksize) {

                            $chunksize = (int) hexdec(trim(ltrim($line, '0'))) + 2;

                        }else{

                            if(strlen($chunked) < $chunksize){

                                $chunked .= $line;

                            }else{

                                $this->buffer .= substr($chunked, 0, $chunksize - 2);
                                $chunksize = (int) hexdec(trim(ltrim($line, '0'))) + 2;
                                $chunked = '';
                            }

                        }

                    }else{

                        $this->buffer .= $line;

                    }
                }


            }

        }

        return (string) $this->buffer;

    }

    public function get($url, $cookie = null) {

        return $this->build(array('GET', $url, null, $cookie));

    }

    public function post($url, $data = null, $cookie = null) {

        return $this->build(array('POST', $url, $data, $cookie));

    }

    public function __set($attr, $value) {

        $this->$attr = $value;

    }

    public function __destruct() {

        if(is_resource(self::$context)) fclose(self::$context);
        unset($this->headerList, $this->header, $this->response, $this->request, $this->args, $this->buffer);

    }

}
/*
HttpClient::init($HttpClient, array('userAgent' => $_SERVER['HTTP_USER_AGENT'], 'redirect' => true));
$HttpClient->get('https://api.weixin.qq.com/sns/userinfo?access_token=ACCESS_TOKEN&openid=OPENID&lang=zh_CN');
echo $HttpClient->buffer;
$json = $HttpClient->buffer;
$json = json_decode($json); 
var_dump($json);
echo $json->errcode;
 */
?>
