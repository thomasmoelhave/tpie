// JSON-P callback
// https://api.github.com/repos/thomasmoelhave/tpie/events?callback=tpieevents
function tpieevents(input) {
    var data = input.data;
    var htmlitems = [];
    for (var i = 0, l = data.length; i < l; ++i) {
        handleevent(data[i], htmlitems);
        if (htmlitems.length >= 5) break;
    }
    var eventhtml = htmlitems.length ? '<li>'+htmlitems.join('</li><li>')+'</li>' : '';
    var activity = document.getElementById('githubactivity');
    activity.innerHTML = eventhtml;
}

function handleevent(ev, html) {
    switch (ev.type) {
        case "PushEvent":
            html.push(handleanyevent(ev, 'pushed some commits to '+printref(ev.payload.ref)));
            break;
        case "CreateEvent":
            var name = ev.payload.ref;
            if (ev.payload.ref_type == 'branch') name = branchlink(name);
            html.push(handleanyevent(ev, 'created '+ev.payload.ref_type+' '+name));
            break;
    }
}

// ISO 8601 specifies YYYY-MM-DDTHH:MM:SSZ
function printdate(iso8601) {
    var d = new Date(iso8601);
    var now = new Date;
    if (d.getFullYear() != now.getFullYear() || d.getMonth() != now.getMonth() || d.getDate() != now.getDate()) {
        return 'on '+d.toLocaleDateString();
    }
    return 'on '+d.toLocaleTimeString();
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
    return [
        '<a href="https://github.com/',ev.actor.login,'">',ev.actor.login,'</a> ',
        desc,
        ' <span class="date">',printdate(ev.created_at),'</span>',
    ''].join('');
}

// vim:set sw=4 sts=4 et:
