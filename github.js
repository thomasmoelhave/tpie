if (!window.console || !console.log)
    console = {log: function () {}}; // all your logs are belong to /dev/null

// JSON-P callback
// https://api.github.com/repos/thomasmoelhave/tpie/events?callback=tpieevents
function tpieevents(input) {
    store_remote_events(input);
    parse_events(input);
}

function parse_events(input) {
    var data = input['data'];
    var htmlitems = [];
    for (var i = 0, l = data.length; i < l; ++i) {
        handleevent(data[i], htmlitems);
        if (htmlitems.length >= 5) break;
    }
    var eventhtml = htmlitems.length ? htmlitems.join('\n') : '';
    var activity = document.getElementById('githubactivity');
    activity.innerHTML = eventhtml;
}

// Check if two authors are equal.
function author_equal(a, b) {
    return a['name'] == b['name'] && a['email'] == b['email'];
}

// If all authors in the array of commits are equal, return the author.
// Otherwise, return null.
function same_author(commits) {
    if (commits.length == 0) return null;
    for (var i = 1, l = commits.length; i < l; ++i) {
        if (!author_equal(commits[i-1]['author'], commits[i]['author'])) return null;
    }
    return commits[0]['author'];
}

// If all authors in a push are the same, use the author's Git realname.
// Otherwise, use the GitHub user name.
function abbreviate_push(ev) {
    var pl = ev['payload'];
    var author = same_author(pl['commits']);
    if (author == null)
        return handleanyevent(ev, 'pushed to '+printref(pl['ref']));
    ev['actor']['realname'] = author['name'];
    return handleanyevent(ev, 'committed to '+printref(pl['ref']));
}

function handleevent(ev, html) {
    var pl = ev['payload'];
    switch (ev['type']) {
        case "PushEvent":
            html.push(abbreviate_push(ev));
            var commits = pl['commits'];
            for (var i = 0, l = commits.length; i < l; ++i) {
                if (i > 2 && l > 4) {
                    html.push("and "+(l-i)+" more commits");
                    break;
                }
                var commit = commits[i];
                html.push(['<li class="commit"><a href="https://github.com/thomasmoelhave/tpie/commit/',commit.sha,'">',
                          commit.sha.substring(0,7),'<\/a> ',
                          commit.message.substring(0,70).replace(/&/g,'&amp;').replace(/</g,'&lt;'),
                          '</li>'].join(''));
            }
            break;
        case "CreateEvent":
        case "DeleteEvent":
            var name = pl['ref'];
            if (pl['ref_type'] == 'branch') name = branchlink(name);
            var verbs = {
                'CreateEvent': 'created',
                'DeleteEvent': 'deleted'
            };
            html.push(handleanyevent(ev, verbs[ev['type']]+' '+pl['ref_type']+' '+name));
            break;
        case "IssueCommentEvent":
            var pl = ev['payload'];
            var issuedata = pl['issue'];
            var url = issuedata['html_url'];
            var issue_number = issuedata['number'];
            var desc = 'commented on <a href="'+url+'">issue '+issue_number+'</a>';
            html.push(handleanyevent(ev, desc));
            break;
        default:
            console.log("Skipping event type ["+ev['type']+"]");
            break;
    }
}

// Calculate the user's local time zone
function calc_tz() {
    // In Firefox, Date.toString() returns something like:
    // Thu May 31 2012 11:37:42 GMT+0200 (CEST)
    var s = ""+(new Date);
    var re = new RegExp("\\(([A-Z]{3,4})\\)", "g");
    var o = re.exec(s);
    if (o)
        return o[1];
    else
        return '';
}

var tz = calc_tz();

// ISO 8601 specifies YYYY-MM-DDTHH:MM:SSZ
function printdate(iso8601) {
    var d = new Date(iso8601);
    var now = new Date;
    var a = '<span title="'+d+'">', b = '</span>';
    if (d.getFullYear() != now.getFullYear() || d.getMonth() != now.getMonth() || d.getDate() != now.getDate()) {
        return a+'on '+d.toLocaleDateString()+b;
    }
    return a+'on '+d.toLocaleTimeString()+' '+tz+b;
}

function branchlink(branch) {
    return '<a href="https://github.com/thomasmoelhave/tpie/tree/'+branch+'">'+branch+'</a>';
}

function printref(refname) {
    if (refname.substring(0, 11) == 'refs/heads/') {
        var branch = refname.substring(11, refname.length);
        return branchlink(branch);
    }
    return refname;
}

function handleanyevent(ev, desc) {
    var actor = ev['actor'];
    return [
        '<li><a href="https://github.com/',actor['login'],'">',(actor['realname'] || actor['login']),'</a> ',
        desc,
        ' <span class="date">',printdate(ev['created_at']),'</span></li>',
    ''].join('');
}

function remote_load() {
    var sc = document.createElement('script');
    sc.type = 'text/javascript';
    sc.src = 'https://api.github.com/repos/thomasmoelhave/tpie/events?callback=tpieevents';
    document.getElementsByTagName('head')[0].appendChild(sc);
}

function store_remote_events(input) {
    if (!localStorage) return false;
    var data = {'input': input, 'cachetime': new Date().getTime()};
    localStorage.setItem('tpieevents', JSON.stringify(data));
}

function fetch_cached_remote_events() {
    if (!localStorage) return false;
    var str = localStorage.getItem('tpieevents');
    if (!str) return false;
    var data = JSON.parse(str);
    if (!data) return false;
    if (!data['input'] || !data['cachetime']) return false;
    parse_events(data['input']);
    var age = new Date().getTime() - data['cachetime'];
    console.log("Age "+age);
    if (age < 0 || age > 30000) return false;
    return true;
}

window.addEventListener('load', function () {
    if (!fetch_cached_remote_events()) {
        console.log("cache miss");
        remote_load();
    } else {
        console.log("cache hit");
    }
}, false);

// vim:set sw=4 sts=4 et:
