/*
function linkclasstag_add(sid, result) {
	if(result) {
		//linkclassform_1=>linkclassform_1_tag
		var obj = $obj(sid+'_tag');
		var newli = document.createElement("h4");
		var x = new Ajax();
		x.get('do.php?ac=ajax&op=linkclasstag', function(s){
			newli.innerHTML = s;
		});
		obj.appendChild(newli);		
	}
}
function linkclass_add(sid, result) {
	if(result) {
		//linkclassform_1=>linkclassform_1_tag
		var obj = $obj(sid+'_name');
		var newli = document.createElement("h4");
		var x = new Ajax();
		x.get('do.php?ac=ajax&op=linkclass', function(s){
			newli.innerHTML = s;
		});
		obj.appendChild(newli);
		
	}
}
*/



/*
function setSearchTab(a) {
        //setCookie(getSearchTabCookieName(), a, 1E3 * 3600 * 24 * 5)
		Cookie.set(getSearchTabCookieName(), a, 1E3 * 3600 * 24 * 5);
    }

function getSearchTabByIndex(a) {
        a = a ? a : 0;
        a = getCookie(getSearchTabCookieName());
        if (a == null || a == "") return 1;
        return a
    };

function getCookie(a) {
     //   var b;
     //   var c = new RegExp("(^| )" + a + "=([^;]*)(;|$)");
     //   if (b = document.cookie.match(c)) return unescape(b[2]);
      //  else return null ;
		return Cookie.get(a);
    };

function setCookie(a, b) {
        var c = arguments[2] ? arguments[2] : 7 * 24 * 60 * 60 * 1000;
        var d = new Date;
        d.setTime(d.getTime() + c);
        var e = "tanzhi.com";
        document.cookie = a + "=" + escape(b); + ";expires=" + d.toGMTString() + ";path=/;domain=" + e ;
    };


function delCookie(a) {
        var b = -1,
            cval = getCookie(a);
        var c = new Date;
        c.setTime(c.getTime() + b);
        if (cval != null) {
                var d = "hao.360.cn";
                document.cookie = a + "=" + escape(cval) + ";expires=" + c.toGMTString() + ";path=/;domain=" + d;
                document.cookie = a + "=" + escape(cval) + ";expires=" + c.toGMTString() + ";path=/;"
            }
    };
*/


/*validate*/

function email_validate(value)
{
	return /^((([a-z]|\d|[!#\$%&'\*\+\-\/=\?\^_`{\|}~]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])+(\.([a-z]|\d|[!#\$%&'\*\+\-\/=\?\^_`{\|}~]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])+)*)|((\x22)((((\x20|\x09)*(\x0d\x0a))?(\x20|\x09)+)?(([\x01-\x08\x0b\x0c\x0e-\x1f\x7f]|\x21|[\x23-\x5b]|[\x5d-\x7e]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(\\([\x01-\x09\x0b\x0c\x0d-\x7f]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF]))))*(((\x20|\x09)*(\x0d\x0a))?(\x20|\x09)+)?(\x22)))@((([a-z]|\d|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(([a-z]|\d|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])*([a-z]|\d|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])))\.)+(([a-z]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(([a-z]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])*([a-z]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])))\.?$/i.test(trim(value));
}
function url_validate(value)
{
	return  /^(https?|ftp):\/\/(((([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(%[\da-f]{2})|[!\$&'\(\)\*\+,;=]|:)*@)?(((\d|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.(\d|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.(\d|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.(\d|[1-9]\d|1\d\d|2[0-4]\d|25[0-5]))|((([a-z]|\d|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(([a-z]|\d|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])*([a-z]|\d|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])))\.)+(([a-z]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(([a-z]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])*([a-z]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])))\.?)(:\d*)?)(\/((([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(%[\da-f]{2})|[!\$&'\(\)\*\+,;=]|:|@)+(\/(([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(%[\da-f]{2})|[!\$&'\(\)\*\+,;=]|:|@)*)*)?)?(\?((([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(%[\da-f]{2})|[!\$&'\(\)\*\+,;=]|:|@)|[\uE000-\uF8FF]|\/|\?)*)?(\#((([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(%[\da-f]{2})|[!\$&'\(\)\*\+,;=]|:|@)|\/|\?)*)?$/i.test(trim(value));
}
function number_validate(value)
{
	return /^\d+$/.test(trim(value));
}
function range_validate(value,min,max)
{
	return ((/^\d+$/.test(trim(value)))&&(parseInt(value)<=max)&&(parseInt(value)>=min));
}
function illegachar_validate(value)
{
   return /[\\/:*?\"<>|]/i.test(trim(value)) ; 
}
function maxlen_validate(value,max)
{
  return(strlen(trim(value))<=max);
}
function rangelen_validate(value,min,max)
{
  return((strlen(trim(value))<=max)&&(strlen(trim(value))>=min));
}
/*
 include UTF-8
*/
function getLength(value)
{
	value=trim(value)
	var length = value.length;
    for(var i = 0; i < value.length; i++){
        if(value.charCodeAt(i) > 127){
            length++;
        }
    }
	return length;
}

function check_subject(id, min, max, specialchar)
{
	if ($('#'+id)) {
		if(!rangelen_validate($('#'+id).val(),min,max))    	
		{
            warning((id+'_tip'),"标题长度("+min+"~"+max+"字符)不符合要求");
            return false;
        }
		if(!specialchar&&illegachar_validate($(id).value))//不支持特殊字符
		{
		   warning((id+'_tip'),"标题不允许含有特殊字符");
		   return false;
		}	
    }
	return true;
}
function check_tag(id, min, max)
{
	if ($('#'+id)) {
		if(!rangelen_validate($('#'+id).val(),min,max))    	
		{
            warning((id+'_tip'),"标签("+min+"~"+max+"字符)不符合要求");
            return false;
        }
		if(illegachar_validate($('#'+id).val()))//不支持特殊字符
		{
		   warning((id+'_tip'),"标签不允许含有特殊字符");
		   return false;
		}
	
    }
	return true;
}
function check_description(id, min, max)
{
	if ($('#'+id)) {
		if(!rangelen_validate($('#'+id).val(),min,max))    	
		{
            warning((id+'_tip'),"描述("+min+"~"+max+"字符)不符合要求");
            return false;
        }
    }
	return true;
}

function check_url(id, min, max)
{
	if ($('#'+id)) {
		if(!url_validate($('#'+id).val())||!rangelen_validate($obj('address').value,min,max))    	
			{
				warning((id+'_tip'),"网址("+min+"~"+max+"字符)不符合要求");
				return false;
			}
		
    }
	return true;
}
function check_seccode(obj,id)
{
	if($('#'+id)) {
		$.ajax({
			  type: "GET",
			  url:'cp.php?ac=common&op=seccode&inajax=1&code='+$('#'+id).val(),
			  success:function(data){
				s=trim($(data).find('root').text());
				if(s.indexOf('succeed') == -1) {
					 warning('checkseccode',s);
					 blur();
           			 return false;
				}else {
					obj.form.submit();
					return true;
				}
			  }
		});
    }else{
		obj.form.submit();
		return true;
	}
}
/*
function check_seccode_x(formobj,id)
{
	if($(id)) {
		var code = $obj(id).value;
		var x = new Ajax();
		x.get('cp.php?ac=common&op=seccode&code=' + code, function(s){
			s = trim(s);
			if(s.indexOf('succeed') == -1) {
				 warning($obj('checkseccode'),s);
           		 return false;
			}else {
				formobj.submit();
				return true;
			}
		});
    }else{
		formobj.submit();
		return true;
	}
}
*/
function check_number(id,min,max)
{
	if ($('#'+id)) {
		if(!range_validate($('#'+id).val(),min,max))    	
			{
				warning((id+'_tip'),"请填写一个("+min+"~"+max+")之间的整数合要求");
				return false;
			}
    }
	return true;
}
function check_email(id)
{
	if ($('#'+id)) {
		if(!email_validate($('#'+id).val()))    	
			{
				warning((id+'_tip'),"请填写一个合法的email地址");
				return false;
			}
    }
	return true;
}
function bookmark_validate(obj,seccode_id, subjectlen, dirlen, urlen, specialchar) {
	var    titlelen=subjectlen;
	var	   isdir=$('input:radio[@name=category][@checked]').val();
	if(isdir)
		titlelen=dirlen; 
	if(!check_subject('subject', 1, titlelen, specialchar)||((!isdir)&&!check_url('address',1,urlen)))
		return false;
    if(!check_seccode(obj,seccode_id))
	    return false;
	return true;
}
function announce_validate(formobj,seccode_id) {
   if(!check_seccode(formobj,seccode_id))
	    return false;
	return true;
}
function link_validate(obj,seccode_id, subjectlen,  urlen, specialchar) {

	if(!check_subject('subject',1,subjectlen, specialchar)||!check_url('address',1,urlen)||!check_number('initaward',5000,9900))
			return false;
	if(!check_seccode(obj,seccode_id))
	    return false;
	return true;   
}
function report_validate(obj) {
/*
    if($obj('seccode')) {
		var code = $obj('seccode').value;
		var x = new Ajax();
		x.get('cp.php?ac=common&op=seccode&code=' + code, function(s){
			s = trim(s);
			if(s.indexOf('succeed') == -1) {
				alert(s);
				$obj('seccode').focus();
           		return false;
			} else {
				obj.form.submit();
				return true;
			}
		});
    } else {
     	obj.form.submit();
    	return true;
    }
*/
	if(!check_seccode(obj,'seccode'))
	    return false;
	return true;
}

function login_validate(obj)
{
	if(!check_email('username'))
			return false;
	obj.submit();
   	return true;
}
function digg_validate(obj,seccode_id)
{
	if(!check_subject('subject', 1, 80, 1))
		return false;
	if(!check_url('address',1,512))
		return false;
	if(!check_tag('tag', 1, 20, 0))
		return false;
	if(!check_description('description', 10, 200, 1))
		return false;

	if(!check_seccode(obj,seccode_id))
	    return false;
	return true;
}
