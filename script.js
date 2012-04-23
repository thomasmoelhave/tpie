var colors;

function Colors(col) {
  var self = this;
  self.col = col;
  var $col = self.$col = $(col);
  var hasfocus = false;
  $col.click(function () {
    if (!hasfocus) {
      this.select();
      hasfocus = true;
    }
  }).blur(function () {
    hasfocus = false;
  }).change(function () {
    self.onInput(this.value);
  }).hover(function () {
    this.style.opacity = 0.7;
  }, function () {
    this.style.opacity = 0.1;
  });

  function isnum(n) {
    return n >= 48 && n <= 57 || n == 45 || n == 46;
  }

  function scrollcolor(delta) {
    var t = col.value;
    var pos = col.selectionStart, pos2 = pos;
    while (pos > 0 && isnum(t.charCodeAt(pos-1)))
      pos--;
    while (pos2 < t.length && isnum(t.charCodeAt(pos2)))
      pos2++;
    if (pos == pos2) return;
    var num = parseInt(t.substring(pos, pos2), 10);
    num += (delta > 0) ? 1 : -1;
    colors.setCSS(t.substring(0, pos)+num+t.substring(pos2, t.length));
    col.selectionStart = col.selectionEnd = pos;
  }

  // from http://www.adomas.org/javascript-mouse-wheel/
  function wheel(event) {
    if (!hasfocus) return true;
    var delta = 0;
    if (!event) event = window.event;
    if (event.wheelDelta) {
      delta = event.wheelDelta/120; 
      if (window.opera) delta = -delta;
    } else if (event.detail) {
      delta = -event.detail/3;
    }
    if (delta)
      scrollcolor(delta);
    if (event.preventDefault)
      event.preventDefault();
    event.returnValue = false;
    return false;
  }

  if (window.addEventListener)
    window.addEventListener('DOMMouseScroll', wheel, false);

  window.onmousewheel = document.onmousewheel = wheel;

}
var _jscss = null;
function getjscss() {
  return document.styleSheets[document.styleSheets.length-1];
}
Colors.prototype.onInput = function (rulestring) {
  var jscss = _jscss || getjscss();
  if (jscss.insertRule && jscss.deleteRule) {
    // Mozilla Firefox
    while (jscss.cssRules.length > 0) {
      jscss.deleteRule(0);
    }

    var rules = rulestring.split('} ');
    for (var i = 0, l = rules.length; i < l; ++i) {
      rules[i] += '}';
      jscss.insertRule(rules[i], i);
    }
  } else if (jscss.cssText) {
    // Internet Explorer
    jscss.cssText = rulestring;
  }
};
Colors.prototype.setCSS = function (c) {
  this.col.value = c;
  this.onInput(c);
};
function rand_interval(lo, hi) {
  return lo+Math.round((hi-lo)*Math.random());
}
function format_hsl(h,s,l) {
  return 'hsl('+h+', '+s+'%, '+l+'%)';
}
function rand_hsl() {
  return format_hsl(rand_interval(0,240), rand_interval(50,100), rand_interval(20,50));
}
Colors.prototype.randomize = function () {
  this.setCSS('h1 { color: '+rand_hsl()+' } h2 { color: '+rand_hsl()+' }');
};
$(function () {
  var $col = $('.colour'), col = $col[0];
  if (!col) return;
  colors = new Colors(col);
});

/* vim: set ts=8 sw=2 sts=2 noet : */
