# 写在前面 #
用于测试的mock服务

不需要任何的代码开发，只是几个简单的配置文件，即可实现mock环境的搭建和测试数据的准备。

传输协议层面：支持http/https，支持扩展的自定义传输协议(比如arpc等)

数据格式层面：支持自定义的数据格式，包括json/protobuf/kv/其他自定义格式等

服务层面：支持client/server

# 编译 #

```bash
$ git clone https://github.com/hqin6/imock.git
$ cd imock/rpm
$ ./imock-build.sh
$ rpm -Uvh imock-*.rpm
```

# 配置说明 #
imock的结构很简单，任何一个imock角色，无论是server还是client，都由四部分组成：

* 可执行程序
* config主配置文件
* fmt.xml应用数据格式定义文件
* dat.xml应用数据内容

其中，在使用时，用户只需要关注后面三个文件即可。即一个配置，两个xml。

## 可执行文件 ##

可执行文件包括两个，一个是imock-client，一个是imock-server。均为c++代码编译链接而成的二进制文件。

### imock-server ###
```bash
$ imock-server -h 
Usage: imock-server [OPTION] 

  -c <config file>  default: /home/a/imock/conf/server/imock-server.conf
  -s <start|stop>   default: start

Examples:
  imock-server -s start
  imock-server -c imock-server.conf -s start
```
imock-server已经做了软连接到`/usr/local/bin`目录下。不需要键入可执行程序的全路径。

命令行参数说明：

* `-c <config file>` : 指定使用的主配置文件。默认值：`/home/a/imock/conf/server/imock-server.conf`
* `-s <start | stop>`: 指定命令类型，启动 | 停止。默认值：`start`

为了方便在测试时键入命令的方便简洁，推荐使用默认参数。比如，启动命令，只需要`sudo imock-server`；停止命令，只需要`sudo imock-server -s stop`

### imock-client ###
```bash
$ imock-client -h
imock-client: invalid option -- h

Usage: imock-client [OPTION]
Send query to server and get answer from server

  -a <area name>      the area will be used in config file.
  -c <config file>    default:/home/a/imock/conf/client/imock-client.conf
  -n <send num>       the number of sending all query for a worker.
  -s <send seconds>   the seconds for client workers run.
  -w <workers>        the number of client workers.
  -l <log level>      crit, error, warn, notice, info, debug. default:info
  -p <msec>           pause milliseconds intervals
  -I <qa id>          the qa id which need send
  -i <q id>           the q id which need send in qa

Examples:
  imock-client -c imock-client.conf -a xxx 
  imock-client -c imock-client.conf -a xxx -w 8
```
imock-client已经做了软连接到/usr/local/bin目录下。不需要键入可执行程序的全路径。

命令行参数说明：

* `-a <area name>` : 指定需要启动的模块角色的配置域名称
* `-c <config file>` : 指定使用的配置文件。默认值：`/home/a/imock/conf/client/imock-client.conf`
* `-n <send num>` : 对每个worker进程，需要重复发送的次数。默认值：1。
__注意__：将指定的数据(dat.xml)从头到尾发一遍算一次。如果指定`-n 4`，则将dat.xml从头到尾重复发送4遍
* `-s <send second>` : 指定发送的秒数。当`-s`指定时，`-n`不生效。注意：每将指定的数据(dat.xml)从头到尾发一遍后才判读时间，所以不管`-s`指定多少，发送的遍数肯定是整数
* `-w <workers>` :	启动worker的进程数。默认值：1。注意：当指定该值，则会覆盖配置文件中的workers配置值。在性能/压力测试时，通过该选项能很方便的的调整进程数
* `-l <log level>` :	指定日志级别。可用值：crit, error, warn, notice, info, debug。默认值：info
* `-p <msec>` :	指定每两个请求发送的时间间隔。单位：毫秒。默认值：0
* `-I <qa ID>` :	指定要发送的qa所对应的debugID号（即在`<qa>`节点的id属性值）。如果有多个qa节点具有相同的debugID值，则都会进行发送。如果不指定，则会按顺序对所有qa节点发送。
* `-i <q ID>` :	指定要发送的q所对应的debugID号（即在`<q>`节点的id属性值）。如果有多个q节点具有相同的debugID值，则都会进行发送。如果不指定，则会按照概率发送。

启动命令（例）：`sudo imock-client -a mock1`

停止命令：`ctrl+c`，因为imock-client不会以Daemon的方式运行，所以是即起即停的方式。

## config配置文件 ##
该文件主要配置imock的服务相关的信息。client和server两种角色的配置方式和内容基本一致。为了清晰，下面仍旧分开说明：

### 对于配置一个/多个 mock server
```bash
#此选项配置需要启动的mock模块，多个mock模块使用逗号分隔
#这里的mock模块每一个都必须能找到对应的配置域，即如果按照如下配置，
#则需要能在配置中读取到[mock1] [mock2]两个配置域
#如果有任何一个读取不到，则启动imock-server失败
mocks = mock1, mock2
#系统日志文件，其格式为”文件名 日志级别”(file [debug|info|warn|error])
#日志级别可选配,不配置，则默认日志级别是info
log = /home/a/imock/logs/imock-server.log debug
#进程记录master进程id号的文件
#可选，默认值: /tmp/imock-server.pid
pid_file = /home/a/imock/bin/imock-server.pid
#########以上就是本配置文件的全局配置项#########

#########以下是本配置文件的各个mock模块的配置项#########
#mock模块的配置项，可以直接写到主配置文件里，也可以另写一个文件，通过include配置
#指令包含到主配置里。以下展示了两种配置方式：其中mock1是直接写入主配置中；mock2
#是通过include的方式包含到主配置中。
#推荐使用include的方式，这样在配置多个mock角色时，不会相互干扰。
#include指令必须在所有全局配置项后面，推荐放到文件最后。

#以下是mock1的角色的配置。
#注意，每个角色有一个配置域，用[]括起来，里面填写配置域的名称
#必须和上面mocks配置项配置的名字一致。
#另外，如果配置了域，但是没有在mocks指令里声明，则不会启动该配置域所对应的mock角色
[mock1]
#启动服务的进程数
#可选，默认值: 1
workers = 1
#用来声明应用数据格式的xml文件
format = /home/a/imock/server/data/mock1.fmt.xml
#用来声明应用数据内容的xml文件
data = /home/a/imock/server/data/mock1.dat.xml
#server接收到的原始message写入文件
#格式为 file mode
#其中mode支持：w w+ a a+ 等方式， 可不配置，默认为w
#通过将file设置为off，可关闭message的记录功能
#可选，默认值: off w
message = /home/a/imock/logs/mock1.server.message.txt
#是否开启数据缓存 (on|off)
#该缓存只针对data配置项指定的dat.xml有效，即如果不开启数据缓存，则每次请求，
#服务都会从硬盘重新加载data配置项指定的dat.xml文件。用于在测试时替换测试数据而不
#重启imock服务，实现数据即时更新即时生效。如果对format配置项指定的fmt.xml内容
#进行修改，则需要重启imock服务才能生效。
#可选，默认值: off
cache = on
#日志定义，如果不设置，该area会继承全局的log配置，否则会（在本area）覆盖全局的log配置
log = /tmp/mock1.log debug
#服务使用协议 (http|https)
protocol = http
#以下配置，针对不同的服务使用协议(protocol)有所差异：
#------针对http
#http服务需要监听的端口号
port = 8080
#------针对https
#https服务需要监听的端口号
port = 443
ssl_cert = certificate-chain.pem.txt
ssl_cert_key = private-key.pem.txt
#
######### mock1的配置项结束，下面通过include方式包含mock2的配置 #########
# include指令，不能写在主配置文件的全局配置之前。推荐写到文件最后。
# 支持正则表达式，比如：
# include /home/a/imock/conf/server/*.conf
# include /home/a/imock/conf/*/*.conf
# 每个配置文件，均是一个独立的配置域，格式为：
#          [配置域名]
#          配置项 = 配置值
include /home/a/imock/conf/server/mock2.conf
```
### 对于配置一个/多个 mock client
和server的配置不同，client的主配置没有全局配置项，直接就是每个角色对应的配置域，如下：

```bash
#########以下是本配置文件的各个mock模块的配置项#########
#mock模块的配置项，可以直接写到主配置文件里，也可以另写一个文件，通过include配置
#指令包含到主配置里。以下展示了两种配置方式：其中mock1是直接写入主配置中；mock2
#是通过include的方式包含到主配置中。
#推荐使用include的方式，这样在配置多个mock角色时，不会相互干扰。

#以下是mock1的角色的配置。
#注意，每个角色有一个配置域，用[]括起来，里面填写配置域的名称
[mock1]
#启动服务的进程数
#可选，默认值: 1
workers = 1
#用来声明应用数据格式的xml文件
format = /home/a/imock/client/data/mock1.fmt.xml
#用来声明应用数据内容的xml文件
data = /home/a/imock/client/data/mock1.dat.xml
#日志定义，如果不设置，则会输出到终端
log = /tmp/mock1.log debug
#client接收到的原始message写入文件
#格式为 file mode
#其中mode支持：w w+ a a+ 等方式， 可不配置，默认为w
#通过将file设置为off，可关闭message的记录功能
#可选，默认值: off w
message = /home/a/imock/logs/mock1.client.message.txt
#发送的超时时间，即接收应答的超时时间，单位毫秒
#可选，默认值: 5000
timeout = 5000
#服务使用协议 (http|https)
protocol = http
#以下配置，针对不同的服务使用协议(protocol)有所差异：
#------针对http
#http/https服务需要请求的地址
#可以带有参数或者不带参数
url = localhost:8080/test?a=1&b=2
#------针对http
#http/https服务能接收的响应长度，单位字节，可不填，默认值是20480字节
resp_max_len = 20480

######### mock1的配置项结束，下面通过include方式包含mock2的配置 #########
# include指令，支持正则表达式，比如：
# include /home/a/imock/conf/client/*.conf
# include /home/a/imock/conf/*/*.conf
# 每个配置文件，均是一个独立的配置域，格式为：
#          [配置域名]
#          配置项 = 配置值
include /home/a/imock/conf/client/mock2.conf
```

## 应用数据格式fmt.xml
即在主配置文件里的format配置项指定的文件。

fmt.xml是标准的xml格式，用以指定应用数据的格式。是序列化(serialize)和反序列化(parse)的依据。

fmt.xml以`<fmt>`为根节点，包含多个`<q>`和多个`<a>`节点。即，一个fmt.xml文件支持多种格式的query和多种格式的answer。即：

```xml
<fmt>
    <q fid='q1'>...</q>
    <a fid='a1'>...</a>
    <a fid='a2'>...</a>
    <q fid='q2'>...</q>
</fmt>
```

* q节点，即query节点。对于imock-server来说，即server本身接收到的数据所对应的数据格式；对于imock-client来说，即client本身发送出去的数据所对应的数据格式。
* a节点，即answer节点。对于imock-server来说，即server本身应答的数据所对应的数据格式；对于imock-client来说，即client本身接收到的数据所对应的数据格式。

fmt中q/a节点的fid属性，是为了区分多个q/a节点。dat.xml中的数据，也是通过该属性和fmt.xml中的q/a节点对应起来的。fmt.xml中的所有q节点，其`fid`的值不能重复，同理，所有a节点的`fid`值也不能重复。当然，q节点和a节点的`fid`值可以相同。当只有一个q/a节点时，可以不配置`fid`属性，也推荐这么做。即：当不配置`fid`属性时，默认`fid`的值为空串。如下的配置是合法的：

```xml
<fmt>
    <q>...</q>
    <a>...</a>
    <a fid='a2'>...</a>
    <q fid='q2'>...</q>
</fmt>
```
所有的q/a节点，顺序可以任意。其他节点的名称，全为用户自定义。

imock-client/ imock-server对fmt.xml的配置要求基本一致。为了说明清晰，下面仍旧分开描述。

### 适用于imock-server
q/a节点的配置类似，说明如下：

#### q节点
imock-server的fmt.xml，里面的q节点描述了：作为server，应该如何解析接收到的字符串数据。根据不同的格式，有不同的节点类型。

##### 普通类型节点
即不需要特殊解析的节点。例如：

```xml
// 接收到的字符串整体是一个ad
// 用分号分割，第一个元素是img_path，第二个元素是click_url
<q>
    <ad sub_def_split=';'>
        <img_path/>
        <click_url/>
    </ad>
</q>
// 如果接收到的字符串是http://img.path;http://click.url
// 那么解析结果则是：
<q>
    <ad>
        <img_path>http://img.path</img_path>
        <click_url>http://click.url</click_url>
    </ad>
</q>
```
下面介绍该类型节点所适用的属性：

---

* `sub_def_split=';'`

节点的内部子节点默认分割符为分号。这里的分隔符不仅限于单个字符，可以是多个字符组成的字符串。

---

* `split=','`	

本节点的分隔符是逗号。这里的分隔符不仅限于单个字符，可以使多个字符组成的字符串。另外，如果配置了该值，则该节点的父节点所配置的`sub_def_split`将在本节点的解析中被覆盖。即：

```xml
<ad sub_def_split=';'>
	<title split=','/>
	<img_path/>
	<click_url/>
</ad>
```
则在进行分隔时，title是按照逗号来分隔而不是分号。如果接收到的字符串是：`女装,img;click`
那么按照上述fmt解析出来的结果是：

```xml
<ad>
	<title>女装</title>
	<img_path>img</img_path>
	<click_url>click</click_url>
</ad>
```

该属性主要用于某一个或者几个节点的分隔符特殊的情况。

---

* `repeated_split='|'`

标记本节点是一个repeated节点，即多值节点。重复的字段之间使用竖线分隔。这里的分隔符不仅限于单个字符，可以使用多个字符组成的字符串。例如：

```xml
<ad sub_def_split=';' repeated_split='|'>
	<img_path/>
	<click_url/>
</ad>
```

则表示广告ad字段是一个重复字段。如果接收到的字符串是：
`img1;click1|img2;click2`
那么按照上述fmt解析出来的结果是：

```xml
<ad>
	<img_path>img1</img_path>
	<click_url>click1</click_url>
</ad>
<ad>
	<img_path>img2</img_path>
	<click_url>click2</click_url>
</ad>
```

注意，本属性的值可以为空，即配置：

```xml
<msg repeated_split=''>
    <size split=',' var='$len'/>
    <body length='$len'/>
</msg>
```

表示msg是多值属性（重复字段），但是没有分隔符。

---

* `var='$varname'`

标记将本节点的值赋值给变量varname。变量名必须是以`$`开头。
该属性主要用于imock-server中需要获取请求里的某个字段并需要使用其值。例如：imock-server接收到的请求串包含一个会话id字段，在应答的时候，需要返回该字段，该字段无法预知且每次都不一样（比如和时间相关的），所以需要将请求中的该字段定义为变量，然后在应答中填写（引用）该字段。例如：

```xml
<session_id var='$sid'></session_id>
```

则是将解析出来的session_id字段的值赋值给变量sid

注意：对于repeated字段，如果设置该属性，则变量值保留的是最后一个解析出来的值。例如：如果字段A有123和456两个值，在字段A上设置var属性，则var变量的值将会是456（最后解析出来的那个）。

---

* `length='4'`	

在解析时，将该长度的字符串赋值给本节点。例如：

```xml
<msg>
	<type length='4'/>
	<body/>
</msg>
```

则如果接收到的字符串是：exchhello，那么按照上述fmt解析出来的结果是：

```xml
<msg>
	<type>exch</type>
	<body>hello</body>
</msg>
```

注意，这里length属性的值可以是一个变量。

应用场景（举例）：
请求串里用逗号分隔，第一个字段表示body的长度，第二个字段是body的内容，而整个字段是多值属性，如下：

```xml
<msg repeated_split=''>
	<len split=',' var='$l'/>
	<body length='$l'/>
</msg>
```
---

* `type='[type]'`

type的取值可以是（可以用`|`分割组合使用）：

`INT`：即将字符串直接按照INT读取。比如：对于二进制字符串（注意转义字符）`”\x31\x0\x0\x0abcd…”`，按照INT读取，则是49，即在解析的时候会将固定的前4个字节作为一个int读取。适用场景举例：比如字符串是“长度+内容”，且长度是一个二进制的int（固定长度4个字节）。

`JARR`：只针对json节点有效，表示本节点是个array节点。因为对于json里，默认对于单值节点会变成普通的json节点，对于单值的json数组(array)节点，需要设置该属性。如果需要设置空数组，即`”key”:[]`,可以使用fmt:`<key type='JARR'/>`; dat:`<key>null</key>`

`JLEAF`：只针对json节点有效，表示本节点是个叶子节点。当对节点设置内嵌格式节点时，需要设置该属性。比如，某个Json节点的子节点是一个kv格式的数据。当既是叶子节点，又是数组时，可以使用`type='JARR|JLEAF'`

---

* `from='[where]'`	

标记本字段的值从哪儿获取。where的取值可以是：

`HTTP_METHOD`:HTTP的请求方法，比如GET/POST等

`HTTP_URI_PATH`:HTTP的请求uri，比如请求http://www.imock.xx/fk?id=1, 则uri是”/fk”

`HTTP_URI_QUERY`:HTTP的请求query，比如请求http://www.imock.xx/fk?id=1, 则query是”id=1”

`HTTP_HEAD`:HTTP头部数据，即HTTP的头部key：value数据，比如：Cookie:cna=xxx\r\nUser-Agent:xxx

`NULL`:来源于空，可配合shell使用。比如，用于定义一个节点，该节点用来生成时间变量，该节点值不来源于任何地方（只是内部的shell产生）
`<pvtime var='$pvtime' op='shell' shell_cmd='/tmp/a.sh' from='NULL'/>`

`BODY`:默认值，即请求body。对于HTTP的GET方法，则为空；POST方法则是HTTP的body。其他非HTTP的协议，则是请求数据

`ARPC_METHOD`:arpc的请求方法，即method名字。主要用于在一个service里有相同request和response类型不同method的情况。比如：一个service会有query和update两种方法，而且其request和response的类型一致（比如都是message Person）。这种情况下，就需要区分不同的method来指导dat.xml里的数据序列化（因为fmt.xml和dat.xml里只有消息类型的区别，这两种method对应的消息类型一致，所以需要用method来区分）。

以上多个值可以使用竖线分割。例如：需要同时响应`GET&POST`请求，请求串可能是GET的参数或者POST的参数（在body中），那么可以这么配置：

```xml
<args fmt='k=v' kv_repeated_split='&#x26;' from='HTTP_URI_QUERY|BODY'>
</args>
```
---

* `op='[operate]'`	

标记在继续解析匹配之前或序列化数据后，对本字段进行何种操作。目前支持：

`nocvt`:目前只针对json节点有效。如果不配置该属性（默认方式），则程序会自动将配置里的字符串"null"转换为`null`节点，将全数字转换为`int`，数字+小数点转换为`float`，`true/false`转换为`bool`类型等。在序列化时有效。

`b64enc/b64enc_safe/b64enc_noappend/b64enc_safe_noappend`:对字符串的base64编码，其中\_safe表示url安全的base64编码，\_noappend表示不追加=号

`b64dec/b64dec_safe/b64dec_noappend/b64dec_safe_noappend`:对字符串的base64解码，其中\_safe表示url安全的base64解码，\_noappend表示不追加=号

`urlenc`:对字符串进行urlencode

`urldec`:对字符串进行urldecode
适用场景举例：比如HTTP的GET请求，具有一个u参数，类似：
`http://xxx.com/q?i=1&u=http%3A%2F%2Fxx.com%2Fa%3D1%26b%3D2`
这里如果需要对u进行进一步解析，则需要将u进行urldecode

`trim`：对本节点的值进行trim去空格处理。

`rtrim`：对本节点的值进行rtrim去右空格处理。

`ltrim`：对本节点的值进行ltrim去左空格处理。

`gzip`：对本节点的值进行gzip压缩处理。

`gunzip`：对本节点的值进行gunzip解压缩处理。

`zip`：对本节点的值进行zip压缩处理。

`unzip`：对本节点的值进行unzip解压缩处理。

`esc`：对本节点的值进行转义处理。比如对\x(十六进制)和\(八进制)进行转义。适用于类似strace获取的转义后数据解析。

`shell`：对本节点的值进行shell调用处理。

`head`：配合`shell`属性使用，表示是否需要传递head给脚本。如果配置，则在执行shell时，会将存储有head数据的文件名传递给shell脚本。如`<n op='shell, head' shell_cmd='./a.sh args1'></n>`则在实际调用a.sh时，会是`./a.sh arg1 head_file`。

`body`：配合`shell`属性使用，表示是否需要传递body给脚本。如果配置，则在执行shell时，会将存储有body数据的文件名传递给shell脚本。如`<n op='shell, head, body' shell_cmd='./a.sh args1'></n>`则在实际调用a.sh时，会是`./a.sh arg1 head_file body_file`。注意，这里如果同时指定head和body，则会固定按照head/body的顺序传递参数，而跟在op属性值里的顺序无关。

`nofile`：配合shell属性使用，当指定该属性时，传递给shell_cmd的参数，均不再是文件名，而是实际参数值。在shell脚本里可直接获取而不用通过读文件。注意：如果参数里有二进制数据（里面有`’\0’`），则不能设置该属性，否则会导致读取的参数不全。

`replacevar`：替换变量值，如果指定，会对节点值里以`$`开始的变量进行替换。比如对`${abc}`或者`$abc`进行替换。

`replaceshvar`：配合`shell`属性使用。替换变量值，如果指定，会对`shell_cmd`里以`$`开始的变量进行替换。比如对`${abc}`或者`$abc`进行替换。

`rvalue`：配合shell属性使用，表示是否需要传递本字段值给脚本。如果配置，则在执行shell时，会将存储有本字段数据的文件名传递给shell脚本。如`<n op='shell, rvalue, head, body' shell_cmd='./a.sh args1'></n>`则在实际调用a.sh时，会是`./a.sh arg1 rvalue_file head_file body_file`。注意，这里如果同时指定head/body/rvalue，则会固定按照rvalue/head/body的顺序传递参数，而跟在op属性值里的顺序无关。

`cached`：配合shell属性使用。缓存shell执行的结果。缓存使用shell_cmd为key。

---

* `shell_cmd='[cmd]'`

当指定`op='shell'`时有效，用以表示需要调用的shell命令，可以写入变量，比如`op='shell,replaceshvar' shell_cmd='a.sh $myvar'`。



##### protobuf格式的节点

即可以用pb进行解析的数据节点。通过`fmt='pb'`表示。由于继承自普通节点，所以普通节点的所有属性均适用于pb节点。pb节点有自己新的属性设置，如下：

---

* `message='[name]'`

本pb节点所对应的pb message。即xx.proto里定义的message名字。注意，如果有名字空间，则需要写全名。例如：xx.proto定义如下：

```proto
package proto.xx;
message Request {…}
```
那么需要这么配置：
`<mypbnode message='proto.xx.Request' …/>`

---

* `file='filename'`	

本pb节点所对应的protobuf file。即假如message的定义在xx.proto文件里，则需要定义如下：

```proto
<pb message='proto.xx.Request' file='xx.proto'…/>
```

注意，file属性值可以为文件名（不含路径），配合`inc`属性；或者为含路径的文件名

---

* `inc='path'`

配置在哪些路径下查找本节点所涉及的pb file。即假如xx.proto文件放在/tmp/proto目录下，则需要定义：

```proto
<pb message='proto.xx.Request' file='/tmp/proto/xx.proto' fmt='pb'/>
```
或者

```proto
<pb message='proto.xx.Request' file='xx.proto' inc='/tmp/proto' fmt='pb'/>
```
该属性在如下场景需要用到：当pb file里使用import包含外部pb文件时。比如xx.proto定义如下（其中desc.proto位于/home/a/pbfile/desc.proto）：

```proto
import "pbfile/desc.proto"
package proto.xx;
message Request {…}
```

则需要定义：

```proto
<pb message='proto.xx.Request' file='xx.proto' inc='/tmp/proto:/home/a' fmt='pb'/>
```
或者

```proto
<pb message='proto.xx.Request' fmt='pb' file='/tmp/proto/xx.proto' inc='/home/a'/>
```

当import属性所对应的文件能在file属性所对应的目录下找到，则不需要配置inc属性。即：当上述例子中的desc.proto位于/tmp/proto/pbfile/desc.proto时，则不需要再配置inc属性，可以配置如下：

```proto
<pb message='proto.xx.Request' file='/tmp/proto/xx.proto' fmt='pb'/>
```
或者

```proto
<pb message='proto.xx.Request' file='xx.proto' inc='/tmp/proto' fmt='pb'/>
```

文件搜索规则：按照配置的路径先后顺序进行查找，找到即停止不再往下查找。

_注意_：inc已经默认增加了两个查找路径（最后查找），`/home/a/imock/proto` 和 `/home/a/imock/dev/proto`
建议将用到的pb放入`/home/a/imock/proto`目录下，就可以不用再次配置路径。`/home/a/imock/dev/proto`不建议使用（imock内部使用目录）。

_注意_：pb节点本身配置以后，如果需要对pb里面的任何字段进行配置，则使用该字段的名字作为节点名即可。如：

```xml
// xx.proto内容如下：
// package test
// message Request {  
//    optional string id = 1;
//    message MSG {
//        optional bytes head = 1;
//        optional bytes body = 1;
//    }
//    optional MSG msg = 2;
// }
// 那么我们可以这么配置：
<q>
    <pb fmt='pb' message='test.Request' file='xx.proto'>
        <id var='$id'/>
        <msg>
        		<body repeated_split=';' sub_def_split=','>
                <title/><click/><img/>
				</body>
        </msg>
    </pb>
</q>
// 如上配置，当获取到一个请求串时，会将串使用test.Request进行解析，并将解析得到
// 的pb里的id字段值赋值给变量$id。然后将进一步解析msg字段里的body字段（body
// 字段又会做为一个普通节点继续解析出title/click/img）。
// 注意：所有格式的节点都能嵌套其他任何格式的节点。
// 比如，你可以将上述的body字段继续定义为一个pb节点，
// 或者将id定义为一个普通节点等等。

// 如果对pb里的子字段没有其他特殊的解析要求，那么上述配置可以为：
<q>
    <pb fmt='pb' message='test.Request' file='xx.proto'/>
</q>
```
##### json格式的节点
即可以用json进行解析的数据节点。通过`fmt='json'`表示。由于继承自普通节点，所以普通节点的所有属性均适用于json节点。和pb、普通节点不同，json节点的所有字段都是请求时才能获取。配置较为简单：

```xml
<q>
	<msg fmt='json'>
        <!--这里可以配置json里面的子节点-->
        <title var='$tt'/>
	</msg>
</q>
// 在解析请求串时，会按照json的格式解析msg节点。如果在msg节点里有配置节点，
// 则会继续解析。比如上述配置，将json解析出来以后，如果有title字段，则会将
// 该字段的值赋值给$tt变量；如果没有title字段，那么title字段就为null

// 注意：所有格式的节点都能嵌套其他任何格式的节点。
// 所以，可以在msg节点里继续增加子节点，子节点的子节点等等。
```

##### key/value格式的节点
即可以用kv进行解析的数据节点。通过`fmt='k?v'`表示。其中的`?`表示key和value的分隔符。例如，使用等号分割的，就是`fmt='k=v'`；使用冒号分割的就是`fmt='k:v'`。这里分隔符不仅限于单个字符，即可以定义`fmt='k::v'`，即表示kv是通过`::`两个冒号的来分割的。由于继承自普通节点，所以普通节点的所有属性均适用于kv节点。和json节点类似，kv节点的所有字段都是请求时才能获取。其支持的属性如下：

---

* `kv_repeated_split=';'`

本kv节点的所有key/value对之间的分隔符。例如：请求字符串`a=1;b=2;c=3`，那么需要这么配置：

```xml
<mykvnode kv_repeated_split=';' fmt='k=v' …/>
```

注意：如果同时配置了该属性和`repeated_split`，例如：

```xml
<n kv_repeated_split=';' fmt='k=v' repeated_split='|' …/>
```
请求串如果是：`a=1;b=2|a=2;b=3`，那么会被解析为：

```xml
<n>
	<a>1</a>
	<b>2</b>
</n>
<n>
	<a>2</a>
	<b>3</b>
</n>
```

---

kv节点的配置较为简单，例如：

```xml
<msg fmt=’k=v' kv_repeated_split='&#x26;' from='HTTP_URI_QUERY'>
        <!--这里可以配置kv里面的子节点-->
        <title var='$tt'/>
</msg>
// 在解析请求串时，会按照k=v的格式解析msg节点。如果在msg节点里有配置节点，
// 则会继续解析。比如上述配置，将kv解析出来以后，如果有key为title的字段，则
// 会将该字段的值赋值给$tt变量；如果没有title字段，那么title字段就为null

// 注意：所有格式的节点都能嵌套其他任何格式的节点。
// 所以，可以在msg节点里继续增加子节点，子节点的子节点等等。
```

#### a节点
imock-server的fmt.xml，里面的a节点描述了：作为server，应该如何序列化dat.xml里的a节点，（序列化后的结果将作为应答返回给client端）。和q节点一样，a节点也支持多种格式的节点。

##### 普通类型的节点
即不需要特殊解析的节点。支持的属性和q节点基本一致。如下：

---

* `sub_def_split=';'` ：和q节点一致
* `split=','` ：和q节点一致
* `repeated_split='|'` ：和q节点一致

---

* `type='[type]'`	type的取值可以是（可以用`|`分割组合使用）：

`INT`：即将字符串直接按照INT序列化。比如：对于字符串49，则会被序列化为二进制字符串（注意转义字符）`\x31\x0\x0\x0`，即在解析的时候会将数字字符串序列化为固定的4个字节int。适用场景举例：比如字符串是`长度+内容`，且长度是一个二进制的int（固定长度4个字节）。

`JARR`：只针对json节点有效，表示本节点是个array节点。因为对于json里，默认对于单值节点会变成普通的json节点，对于单值的json数组(array)节点，需要设置该属性。如果需要设置空数组，即`”key”:[]`,可以使用fmt:`<key type='JARR'/>`; dat:`<key>null</key>`

`JLEAF`：只针对json节点有效，表示本节点是个叶子节点。当对节点设置内嵌格式节点时，需要设置该属性。比如，某个Json节点的子节点是一个kv格式的数据。当既是叶子节点，又是数组时，可以使用`type='JARR|JLEAF'`

---

* `to='[where]'`	标记本字段的值需要写入到哪儿。where的取值可以是：

`HTTP_HEAD`:HTTP应答头部数据，即HTTP的头部key：value数据，比如：Location:http://www.imock.xx

`HTTP_VERSION_MINOR`:HTTP的应答版本，目前支持”0”或者”1”，即1.0或者1.1

`HTTP_CODE`:HTTP应答码，比如302、404等。

`HTTP_CODE_MSG`:HTTP应答码对应的消息，比如”Not Found”（即404对应的消息），可以不配置，会自动添加（通过HTTP_CODE获取标准的CODE_MSG）。

`ARPC_FAILED_MSG`:ARPC应答的失败消息。

---

##### protobuf格式的节点
和q节点的属性一致。不再重复说明。
##### json格式的节点
和q节点的属性一致。不再重复说明。
##### key/value格式的节点
和q节点的属性一致。不再重复说明。

### 适用于imock-client
q/a节点的配置类似，说明如下

#### q节点
imock-client的fmt.xml，里面的q节点描述了：作为client，应该如何序列化dat.xml里的q节点。（序列化后的结果将作为应答返回给client端）。根据不同的格式，有不同的节点类型。

##### 普通类型的节点
和imock-server的a节点类似（即都是进行序列化），支持的属性如下：

---

* `sub_def_split=';'`：和imock-server的a节点一致
* `split=','`：和imock-server的a节点一致
* `repeated_split='|'`	和imock-server的a节点一致
* `type='[type]'`	和imock-server的a节点一致

---

* `to='[where]'`	标记本字段的值需要写入到哪儿。where的取值可以是：

`HTTP_METHOD`：HTTP的请求方法，比如GET/POST等

`HTTP_URI_PATH`：HTTP的请求uri，比如请求http://www.imock.xx/fk?id=1, 则uri是”/fk”

_注意_，imock-client发送的请求url是如下生成的：
`conf里的url + HTTP_URI_PATH + HTTP_URI_QUERY`
如果在conf(imock-client.conf)里配置的url本身就具有uri，那么拼接出来的url将会是错误的结果。所以，如果需要用到本属性，那么就不能在conf里对url加uri。比如：
conf里配置的url是:http://www.im.xx, 配置的HTTP_URI_PATH是”/item”，那么最终的url会是：
http://www.im.xx/item. 注意，这里不会在中间加任何分隔符，即如果配置的HTTP_URI_PATH是”item”，那么最终的url会是：http://www.im.xxitem. 

`HTTP_URI_QUERY`：HTTP的请求query，比如请求http://www.imock.xx/item?id=1, 则query是”id=1”
注意，imock-client发送的请求url对该属性的处理是直接将其变为k=v的模式追加到url里。至于`?`和`&`，则会判断conf配置的url里是否有，如果有则不追加，否则追加。例如：
conf里配置的url是：http://www.im.xx?i=1, 配置的`HTTP_URI_QUERY`序列化后是`a=1&b=2`，那么最终的url会是：
`http://www.im.xx?i=1&a=1&b=2`。
如果conf里的配置url是：http://www.im.xx, 那么最终的url会是：`http://www.im.xx?a=1&b=2`

`HTTP_HEAD`：HTTP头部数据，即HTTP的头部key：value数据，比如：Cookie:cna=xxx\r\nUser-Agent:xxx

`HTTP_VERSION_MINOR`：HTTP的版本，目前支持”0”或者”1”，即1.0或者1.1

`BODY`：默认值，即请求body。对于HTTP的GET方法，则为空；POST方法则是HTTP的body。其他非HTTP的协议，则是请求数据

`ARPC_METHOD`：arpc的请求方法，即method名字。

`ARPC_SERVICE`：arpc的请求service，即service名字。需要使用全名。即如果有命名空间，需要包含命名空间。例如：
arp的服务名为 xxx，其包名为：package test。那么需要配置为：test.xxx

---

##### protobuf格式的节点
和imock-server的a节点的属性一致。不再重复说明。
##### json格式的节点
和imock-server的a节点的属性一致。不再重复说明。
##### key/value格式的节点
和imock-server的a节点的属性一致。不再重复说明。

#### a节点
imock-client的fmt.xml，里面的a节点描述了：作为client，应该如何解析其接收到的应答数据。（解析后的结果可以和dat.xml中给出的值进行匹配，可选）。根据不同的格式，有不同的节点类型

##### 普通类型的节点
和imock-server的q节点类似（即都是进行字符串的解析），支持的属性如下：

---

* `sub_def_split=';'`：和imock-server的q节点一致
* `split=','`：和imock-server的q节点一致
* `repeated_split='|'`：和imock-server的q节点一致
* `var='$varname'`：和imock-server的q节点一致
* `length='4'`：和imock-server的q节点一致
* `type='[type]'`：和imock-server的q节点一致
* `op='[operate]'`：	和imock-server的q一致

---

* `from='[where]'`	标记本字段的值从哪儿获取。where的取值可以是（多值，可以用`|`分割)：

`HTTP_HEAD`：HTTP应答头部数据，即HTTP的头部key：value数据，比如：Location:http://www.imock.xx 

`HTTP_VERSION_MINOR`：HTTP的应答版本，目前支持”0”或者”1”，即1.0或者1.1

`HTTP_CODE`：HTTP应答码，比如302、404等。

`HTTP_CODE_MSG`：HTTP应答码对应的消息，比如”Not Found”（即404对应的消息），可以不配置，会自动添加（通过HTTP_CODE获取标准的CODE_MSG）。

`ARPC_FAILED_MSG`：ARPC应答的失败消息。

`BODY`：默认值，即应答body。

---

##### protobuf格式的节点
和imock-server的q节点的属性一致。不再重复说明。
##### json格式的节点
和imock-server的q节点的属性一致。不再重复说明。
##### key/value格式的节点
和imock-server的q节点的属性一致。不再重复说明。


## 应用数据内容dat.xml

即在主配置文件里的data配置项指定的文件。

dat.xml是标准的xml格式，用以指定应用数据的内容。是需要序列化(serialize)和反序列化(parse)的对象。
dat.xml以`<dat>`为根节点，包含多个`<qa>`节点，每个`<qa>`节点包含多个`<q>`和多个`<a>`节点。即：

```xml
<dat>
   <qa>
        <q fid='q1'>...</q>
        <a fid='a1'>...</a>
        <a fid='a2'>...</a>
        <q fid='q2'>...</q>
	</qa>
	<qa>…</qa>
</dat>
```


q节点，即query节点。对于imock-server来说，即和server本身接收到的数据进行匹配的数据内容；对于imock-client来说，即client本身发送出去的数据内容。

a节点，即answer节点。对于imock-server来说，即server本身应答的数据所对应的数据内容；对于imock-client来说，即和client本身接收到的数据进行匹配的数据内容。

dat中q/a节点的`fid`属性，用于和fmt.xml中的q/a节点进行对应。即指明dat.xml中本节点(q/a)需要使用何种格式进行解析。其dat.xml中所指定的`fid`必须都能在fmt.xml中存在。

所有的q/a节点的下属节点（子节点）必须和fmt.xml中对应。字段顺序可以任意。例如：

```xml
<fmt>
<q>
    <title split=';'/>
    <body/>
</q>
<a></a>
<q fid='q1'>
   <msg sub_def_split=':' repeated_split=';'>
        <key/>
        <value/>
	</msg>
</q>
<a fid='a1'>
    <len/>
    <msg/>
</a>
</fmt>
// 对于以上的fmt.xml配置，其对应的dat.xml可以如下：
<dat>
	<qa>
		<q fid='q1'>
            <msg>
                <key>testKey</key>
                <value>testValue</value>
            </msg>
		</q>
		<a fid='a1'>
            <len>5</len>
            <msg>abcde</msg>
		</a>
	</qa>
</dat>
```

_注意_，以下几点同时适用于imock-server和imock-client：

* 对于protobuf里的枚举值、bool值，必须写字面值，即枚举值请填写枚举值的字符串内容（而不是0/1/2等数字）。bool值填写true/false。
* 对于重复字段（不论是通过repeated_split声明，还是protobuf里定义的repeated字段），直接写多个节点即可。例如：

```xml
<msg>
	<body>Body 1 in msg 1</body>
</msg>
<msg>	
	<body>Body 1 in msg 2</body>
	<body>Body 2 in msg 2</body>
</msg>
```

* 对于重复字段的匹配，则是包含关系，即假如dat.xml中配置的重复字段有三个值，只要这三个值都在匹配对象里存在且能逐个匹配即算匹配成功（即使匹配对象里有4个甚至更多的值）。


imock-client/ imock-server对dat.xml的配置要求基本一致。为了说明清晰，下面仍旧分开描述。

### 适用于imock-server

当接收到一个请求串后，imock-server会根据dat.xml返回相应结果。其逻辑如下：

1. 按照顺序，依次从dat.xml中读取`<qa>`节点。
2. 对每一个`<qa>`节点，使用`<q>`节点声明的fid所对应的格式声明（即该fid在fmt.xml中对应的q节点）对请求字符串进行解析。
3.	解析完（不管成功还是失败），会和dat.xml中的该`<q>`节点进行匹配。匹配成功，则将其所在的`<qa>`节点中的`<a>`节点进行序列化返回。其`<a>`节点的序列化方式，也是`<a>`节点声明的fid所对应的格式（即该fid在fmt.xml中对应的a节点）。
4.	如果最终没有找到任何匹配的q节点，则不会返回任何数据给client端。

注意以下几点：

* 对请求字符串的解析，如果解析成功（不管匹配成功与否），后续将不再重新解析。即：假如请求字符串满足多种格式的解析，以第一种格式为准。这里的谁是第一，是按照dat.xml中的q节点对应的fid来决定，而不是按照fmt.xml中的q节点顺序。
* 对于一个`<qa>`节点里的多个q节点，只要任意一个q节点匹配成功，则本`<qa>`节点算是匹配成功，将返回本`<qa>`节点对应的`<a>`节点。
* 对于一个`<qa>`节点里的多个a节点，当需要返回时，则会按照每个a节点配置的rate按概率只选取一个序列化返回。

q/a节点的配置类似，说明如下：

#### q节点
imock-server的dat.xml，里面的q节点描述了：作为server，应该如何对接收到的字符串数据进行匹配。和fmt.xml不同，dat.xml中的所有的节点，都是数据节点。

数据节点支持的属性如下：

---

* `id='xx'`	

给本节点指定一个debugID，这个id只是用来记录日志使用。例如：当一个dat.xml中有多个q节点时，对每个q节点定义一个唯一的id属性，即可从日志中快速定位匹配请求的是哪个q节点。该属性只有在`<q>`节点有效。在其他节点配置则无效。

---

* `match_shell_cmd='xx'`

配合`match='shell'`属性使用。原理同fmt节点的`shell_cmd`属性

---

* `match='xx'`

指定本节点如何匹配。其中xx可以取值如下：

`shell`：自定义shell匹配规则。比如 `<n match='shell' match_shell_cmd='echo –n 1'></n>` 表示n节点的值按照`match_shell_cmd`匹配进行。如果shell命令返回的字符串第一个字符是`0`，则表示匹配成功。如上例子，由于无论如何输出是`1`，则永远匹配失败。

`replaceshvar`：配合shell属性使用。原理同fmt里的`op='replaceshvar'`属性

`reg`：正则匹配。比如 `<n match='reg'>[0-9]*</n>` 表示n节点的值按照正则匹配进行，其正则表达式是`[0-9]*`。如果请求字符串解析出来的`<n>`节点值是`1234567`，那么可以匹配成功。如果是`a123`，则匹配失败。

`icase`：忽略大小写，即在进行match比较前，会将需要比对的两个字符串都转换为小写。

`int`：将需要比对的两个字符串均atoi转换为整型。

`'<', '>', '=', '!='`：进行小于、大于、等于、不等于匹配。比如：`<n match='<'>10</n>` 如果请求字符串解析出来的`<n>`节点值是20，那么匹配失败。如果要按照数字int做比较，则需要配合int使用，例如：`<n match='<, int'>10</n>`

`fmt_err`：匹配解析失败。该属性只有在`<q>`节点才有效，`<q>`子节点无效。

`fmt_ok`：匹配解析成功。即格式正确就算匹配。该属性只有在`<q>`节点才有效，`<q>`子节点无效。

`no_exist`：不存在即匹配。例如在protobuf里，如果某个字段不存在，则匹配成功。

`exist`：存在即匹配。例如在protobuf里，如果某个字段存在，则匹配成功。

_注意_：上述值可以通过逗号分割符进行组合使用。例如：

`<n match='reg, icase'>aa.*</n>`： 忽略大小写的正则

`<q match='fmt_err,fmt_ok'></q>`：正不正确都匹配。

`<n match='<,=,int'>10</n>`：按数字小于等于10匹配

---

* `from='xx'`

指定本节点的值来自何处。其中xx可以取值如下：
`file`：本节点的值来自于文件。如`<n from='file'>a.txt</n>`表示本节点的值来自于a.txt文件的内容。

`var`：本节点的值来自于变量。如`<n from='var'>$id</n>`表示本节点的值来自于变量`$id`的值。

`cached`：本节点的值是否可缓存。需要配合`file/shell`属性使用。为了提升效率，不每次都重新读取文件或者不每次重新调用shell，所以可以加本属性减少file的读取操作或shell的执行次数。比如：`<n from='file,cached'>a.txt</n>`表示本节点的值来自于a.txt文件的内容，且内容只会被读取一次并cached到内存里。注意：这里可以通过逗号分割符进行组合使用，不区分顺序。

---

* `op='xx'`	同fmt节点的op属性

---

#### a节点
imock-server的dat.xml，里面的a节点描述了：作为server，应该应答什么样的数据给client端。和fmt.xml不同，dat.xml中的所有的节点，都是数据节点。

数据节点支持的属性如下：

---

* `id='xx'`	和q节点一致，是debugID，只有在`<a>`节点有效。
* `from='xx'`	和q节点一致。

---

* `op='xx'`	除了继承所有fmt节点的op属性外，还有：

`hide`：隐藏本节点。主要应用于普通格式，比如使用`^A`分割的字符串。如果需要模拟少一个`^A`（缺一个字段），则需要使用该属性

`no_pack`：不序列化本字段，即将本节点的值直接返回。例如：`<n op='no_pack'>error format</n>`则会将“error format”作为内容返回，而不关心n节点的格式

---

* `rate='整型'`

只有在`<a>`节点有效。当一个`<qa>`节点里有多个`<a>`节点，则如果不配置rate属性，则多个`<a>`是随机按相同概率选取一个`<a>`节点作为应答。如果需要配置按照不同比例返回`<a>`节点，则需要配置rate。对于同一个`<qa>`节点里的所有`<a>`节点所配置的rate，会做归一化处理。即：如果有三个`<a>`节点，配置的rate分别是2、4、6，则会按照2:4:6的比例进行选取。

---

* `sleep='xx'`	

xx的取值举例：10m：10分钟；1s：1秒；100ms：100毫秒；500us：500微秒；只有在`<a>`节点有效。主要是为了模拟应答的延时。

---

* `repeated_num='xx'`	

xx的取值，可以是变量，可以是整型数字。该属性主要用于需要动态确定repeated字段的个数的场景下。比如，请求有ids=1,2,3,4，需要根据请求里的ids个数和内容组织应答。可以在解析请求时，对ids字段进行shell处理，将数据存储到临时文件中，并将个数保存在变量中。在应答时，逐个读取临时文件里的值即可。例如:

```xml
<ids op='shell' shell_cmd='./cnt.sh' var='$n'></ids>
```
其中，./cnt.sh脚本内容可以：

```bash
num=$(echo "$1" | tr , \\n  | tee /tmp/a | wc -l);
```

在应答里可以：

```xml
<n repeated_num='$n' op='shell'  shell_cmd='./get.sh'></n>
```

其中，./get.sh脚本内容可以：

```bash
head -1 /tmp/a  && sed -i '1d'
```
---

### 适用于imock-client
imock-client会根据dat.xml中的内容，向server发送请求数据。其逻辑如下：

1.	按照顺序，依次从dat.xml中读取`<qa>`节点。
2.	对每一个`<qa>`节点，使用`<q>`节点声明的fid所对应的格式声明（即该fid在fmt.xml中对应的q节点）对该节点进行序列化。
3.	将序列化的字符串发送给server。
4.	接收server的应答（如果有）。并对应答的字符串进行解析并匹配`<a>`节点。其序列化和匹配的方式和imock-server的`<q>`节点匹配类似。如果匹配成功，则以0为退出码退出，否则以非0为退出码退出。

_注意_：

* 对于一个`<qa>`节点里的多个a节点，只要任意一个a节点匹配成功，则本`<qa>`节点算是匹配成功，将以0为退出码退出，否则以非0退出码退出。
* 对于一个`<qa>`节点里的多个q节点，会按照每个q节点配置的rate按概率只选取一个序列化发送。

q/a节点的配置类似，说明如下：

#### q节点

imock-client的dat.xml，里面的a节点描述了：作为client，应该发送什么样的数据给server端。和fmt.xml不同，dat.xml中的所有的节点，都是数据节点。

数据节点支持的属性如下：

* `id='xx'`：和imock-server的a节点一致，是debugID，只有在`<q>`节点有效。
* `from='xx'`：和imock-server的a节点一致。
* `op='xx'`	和imock-server的a节点一致。
* `rate='整型'`	和imock-server的a节点一致。
* `repeated_num='整型'`	和imock-server的a节点一致。

#### a节点
imock-client的dat.xml，里面的a节点描述了：作为client，应该如何对接收到的应答字符串数据进行匹配。和fmt.xml不同，dat.xml中的所有的节点，都是数据节点。

数据节点支持的属性如下：

* `id='xx'`：和imock-server的q节点一致
* `match='xx'`：和imock-server的q节点一致
* `from='xx'`：和imock-server的q节点一致
* `op='xx'`：和imock-server的q节点一致

## 对某些属性的特殊说明
由于属性偏多，而且有些属性既在server里用，又在client里用；在fmt里用，也在dat里用；在q节点里用，也在a节点里用。对于有些属性说明如下：

### op属性
op属性即指定节点的具体操作，这里的操作:

1.	对于fmt节点而言，如果是解析，则发生在解析本节点前对字符串的处理；如果是序列化，则发生在序列化本节点值以后。
2.	对于dat节点而言，(不存在解析的情况)。如果是匹配，则在匹配之前会对dat节点（在dat.xml中的而不是实时解析request出来的）调用；如果是序列化，则发生在序列化本节点值以后。
3.	和shell操作配合使用的rvalue是请求/应答的dat节点值，而非dat.xml里配置的。
4.	op支持链表操作。即op=’esc, gunzip’表示先进行转义后接着进行gzip解压。


# 应用实例
## 普通数据
### 场景描述
client发送给server的数据格式如下：

client→server 查询广告字符串：
		`querysn?ip^Bcookie^Aurl^Aadnum`
		
例如：
`querysn?127.0.0.1^BDe16DbsE50YCAbZc^Awww.test.com^A3`

server→client 应答数据：
`status^Aadboardid,rate^Badboardid,rate^B…`

例如：`0^A487123001,30^B487123002,20^B487123003,50`

### client配置

```bash
#/home/a/imock/conf/client/imock-client.conf
[queryAd]
workers = 1
...
format = /home/a/imock/data/client/queryAd-fmt.xml
data = /home/a/imock/data/client/queryAd-dat.xml
message = /tmp/client-msg.txt w
```

```xml
<!--/home/a/imock/data/client/queryAd-fmt.xml--> 
<fmt>
    <q sub_def_split='&#1;'>
        <head split='?'/>
        <user sub_def_split='&#2;'>
            <ip/>
            <cookie/>
        </user>
        <url/>
        <adnum/>
	</q>
	<a>
    	<status split='&#1;'/>
    	<info repeated_split='&#2;'>
        	<adbid split=','/>
        	<rate/>
    	</info>
	</a>
</fmt>

<!--/home/a/imock/data/client/queryAd-dat.xml--> 
<dat>
	<qa>
    	<q>
            <head>querysn</head>
            <user>
                <ip>127.0.0.1</ip>
            </user>
            <url>www.test.com</url>
            <adnum>3</adnum>
		</q>
		<a>
   	 		<status>0</status>
		</a>
	</qa>
</dat>

```

### server配置

```bash
#/home/a/imock/conf/server/imock-server.conf
mocks = queryAd
log = /home/a/imock/logs/error.log

[queryAd]
workers = 1
...
format = /home/a/imock/data/client/queryAd-fmt.xml
data = /home/a/imock/data/server/queryAd-dat.xml
message = /tmp/server-msg.txt
```

```xml
<!—和client共用 /home/a/imock/data/client/queryAd-fmt.xml--> 
<!--/home/a/imock/data/server/queryAd-dat.xml--> 
<dat>
	<qa>
    	<q>
            <head>querysn</head>
            <user>
                <ip>127.0.0.1</ip>
            </user>
		</q>
		<a>
    		<status>0</status>
    		<info>
        		<adbid>487123001</adbid>
        		<rate>30</rate>
    		</info>
    		<info>
        		<adbid>487123002</adbid>
        		<rate>20</rate>
    		</info>
    		<info>
        		<adbid>487123003</adbid>
        		<rate>50</rate>
    		</info>
		</a>
	</qa>
</dat>
```

### 测试

```bash
$sudo imock-server
$sudo imock-client –a queryAd
$cat /tmp/server-msg.txt  -v
querysn?127.0.0.1^B^Awww.test.com^A3
$cat /tmp/client-msg.txt  -v
0^A487123001,30^B487123002,20^B487123003,50
```

## http+kv数据

### 场景描述
client通过http发送给server的数据格式如下：

client→server 查询广告字符串：`ip=xx&url=xx`

例如：
`ip=127.0.0.1&url=http%3A%2F%2Fabc.com%2Fa%3D1%26f%3Da.txt`

server→client 应答数据为：返回url参数例的f参数所指定的文件内容。如上示例就是a.txt

注意：client发送给server的有可能是GET请求，也可能是POST请求

### client配置

```bash
#/home/a/imock/conf/client/imock-client.conf
[http_queryFile]
workers = 1
protocol = http
url = localhost:9888/query
format = /home/a/imock/data/client/http_queryFile-fmt.xml
data = /home/a/imock/data/client/http_queryFile-dat.xml
message = /tmp/client-msg.txt w
```

```xml
<!--/home/a/imock/data/client/http_queryFile-fmt.xml--> 
<fmt>
    <q>
        <getArg fmt='k=v' kv_repeated_split='&#38;' to='HTTP_URI_QUERY'>
            <url>
                <uri split='?'/>
                <args fmt='k=v' kv_repeated_split='&#38;'/>
			  </url>
  		  </getArg>
        <pstArg fmt='k=v' kv_repeated_split='&#38;'>
            <url>
                <uri split='?'/>
                <args fmt='k=v' kv_repeated_split='&#38;'/>
			  </url>
        </pstArg>
	</q>
	<a/>
</fmt>

<!--/home/a/imock/data/client/http_queryFile-dat.xml--> 
<dat>
	<qa>
    	<q>
            <getArg>
                <ip>127.0.0.1</ip>
                <url op='urlenc'>
                    <uri>http://abc.com</uri>
                    <args>
                        <a>1</a>
                        <f>a.txt</f>
						</args>
					</url>
				</getArg>
		</q>
		<a match='fmt_ok'/>
	</qa>
	<qa>
    	<q>
            <pstArg>
                <ip>127.0.0.1</ip>
                <url op='urlenc'>
                    <uri>http://abc.com</uri>
                    <args>
                        <a>1</a>
                        <f>a.txt</f>
						</args>
					</url>
				</pstArg>
		</q>
		<a match='fmt_ok'/>
	</qa>
</dat>

```

### server配置

```bash
#/home/a/imock/conf/server/imock-server.conf
mocks = http_queryFile
log = /home/a/imock/logs/error.log

[http_queryFile]
workers = 1
protocol = http
port = 9888
format = /home/a/imock/data/server/http_queryFile-fmt.xml
data = /home/a/imock/data/server/http_queryFile-dat.xml
message = /tmp/server-msg.txt
```

```xml
<!--/home/a/imock/data/server/http_queryFile-fmt.xml--> 
<fmt>
    <q>
        <Arg fmt='k=v' kv_repeated_split='&#38;' from='HTTP_URI_QUERY|BODY'>
            <url op='urldec'>
                <uri split='?'/>
                <args fmt='k=v' kv_repeated_split='&#38;'>
                    <f var='$fileName'/>
                </args>
			</url>
		</Arg>
	</q>
	<a/>
</fmt>
<!--/home/a/imock/data/server/http_queryFile-dat.xml--> 
<dat>
	<qa>
    	<q>
        <Arg>
            <url op='urldec'>
                <args>
                    <f match='exist'/>
					</args>
            </url>
        </Arg>
		</q>
		<a from='file,cached'>$fileName</a>
	</qa>
</dat>
```
### 测试

```bash
$echo “content in file” > a.txt
$sudo imock-server
$sudo imock-client –a http_queryFile
```

## arpc+protobuf数据

### 场景描述
client通过arpc和server的protobuf定义如下（addressbook.proto）：

```proto
import "arpc/proto/rpc_extensions.proto";
package tutorial;
option java_package = "com.example.tutorial";
option java_outer_classname = "AddressBookProtos";
option cc_generic_services = true;
option java_generic_services = true;
message Person {
  required string name = 1;
  optional int32 id = 2;
  optional string email = 3;
  enum PhoneType {
    MOBILE = 0;
    HOME = 1;
    WORK = 2;
  }
  message PhoneNumber {
    required string number = 1;
    optional PhoneType type = 2 [default = HOME];
  }
  repeated PhoneNumber phone = 4;
}
message ACK {
  optional string message = 1 [default = "OK"];
}
service Foo {
  option (arpc.global_service_id) = 65535;
  rpc Query(Person) returns(Person) {
    option (arpc.local_method_id) = 32767;
  }
  rpc Update(Person) returns(ACK) {
    option (arpc.local_method_id) = 32766;
  }
  rpc Delete(Person) returns(ACK) {
    option (arpc.local_method_id) = 32765;
  }
}
```

### client配置

```bash
#/home/a/imock/conf/client/imock-client.conf
[arpc_query]
workers = 10
protocol = arpc
address = tcp:localhost:9876
proto_file = /home/a/imock/proto/addressbook.proto
format = /home/a/imock/data/client/arpc_query-fmt.xml
data = /home/a/imock/data/client/arpc_query-dat.xml
message = /tmp/client-msg.txt w
```

```xml
<!--/home/a/imock/data/client/arpc_query-fmt.xml--> 
<fmt>
    <q>
        	<service to='ARPC_SERVICE'/>
			<method to='ARPC_METHOD'/>
			<pb fmt='pb' message='tutorial.Person' file='addressbook.proto'>
			</pb>
	 </q>
	 <a fid='ack'>
	 	<pb fmt='pb' message='tutorial.ACK' file='addressbook.proto'>
	 	</pb>
	 </a>
	 <a>
		<pb fmt='pb' message='tutorial.Person' file='addressbook.proto'>
		</pb>
	 </a>
</fmt>

<!--/home/a/imock/data/client/arpc_query-dat.xml--> 
<dat>
<qa>
    <q>
        <service>tutorial.Foo</service>
        <method>Update</method>
            <pb>
                <name>zhangsan</name>
                <phone>
                    <number>010-12345678</number>
                    <type>HOME</type>
				   </phone>
                <phone>
                    <number>13888888888</number>
                    <type>MOBILE</type>
				  </phone>
			 </pb>
	</q>	
	<a fid='ack' match='fmt_ok'/>
</qa>
<qa>
    <q>
        <service>tutorial.Foo</service>
        <method>Query</method>
            <pb>
                <name>zhangsan</name>
                <phone>
                    <number>010-12345678</number>
                    <type>HOME</type>
				   </phone>
                <phone>
                    <number>13888888888</number>
                    <type>MOBILE</type>
					</phone>
				</pb>
	 </q>
	 <a match='fmt_ok'/>
</qa>
</dat>

```


### server配置

```bash
#/home/a/imock/conf/server/imock-server.conf
mocks = arpc_query
log = /home/a/imock/logs/error.log

[arpc_query]
workers = 10
protocol = arpc
address = tcp:localhost:9876
proto_file = /home/a/imock/proto/addressbook.proto
format = /home/a/imock/data/server/arpc_query-fmt.xml
data = /home/a/imock/data/server/arpc_query-dat.xml
message = /tmp/server-msg.txt
```

```xml
<!--/home/a/imock/data/server/arpc_query-fmt.xml--> 
<fmt>
	<q>
		<pb fmt='pb' message='tutorial.Person' file='addressbook.proto'>
		</pb>
		<method from='ARPC_METHOD'/>
	</q>
	<a fid='ack'>
		<pb fmt='pb' message='tutorial.ACK' file='addressbook.proto'>
		</pb>
	</a>
	<a>
		<pb fmt='pb' message='tutorial.Person' file='addressbook.proto'>
		</pb>
	</a>
</fmt>

<!--/home/a/imock/data/server/arpc_query-dat.xml--> 
<dat>
<qa>
    <q>
        <method>Query</method>
	</q>
	<a>
    <pb>
        <name>lisi</name>
	 </pb>
	</a>
</qa>
<qa>
    <q>
        <method>Update</method>
	</q>
	<a>
    <message>update ok.</message>
	</a>
</qa>
</dat>

```

### 测试

```bash
$sudo imock-server
$sudo imock-client –a arpc_query
```

## json数据支持
类似kv数据，暂不详细介绍

## 关于性能测试
对于性能测试，为了提高其imock的效率，可以进行如下优化：

* 增加workers配置的线程/进程数
* log的日志级别可以提高到error
* message可以配置为off，避免每次请求读写磁盘
* cache配置为on，可以避免每次请求时重新载入dat.xml
* 尽量减少匹配的复杂度，比如可以使用`match=’fmt_ok’`

# 接口API
为了方便扩展和功能复用，imock提供了.h和.a/.so提供用户进行自定义编程。请参考：
/home/a/imock/dev-interface/include/imock-interface.h
/home/a/imock/dev-interface/libs/libimock-interface.a
/home/a/imock/dev-interface/libs/libimock-interface.so


imock-parse

imock-serialize

imock-diff：该工具旨用于对比不同的原始字符串文件，比如新老两份日志文件（格式可以不同）。该工具的使用，需要注意配置文件的格式。详见安装包里的示例。

