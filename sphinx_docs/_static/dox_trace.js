// noinspection JSUnusedGlobalSymbols

let visibleTextFound = false
function visibleText(elem) {
    $.each($(elem).contents(), function(i, itm) {
        if (visibleTextFound)
            return false
        if (itm.nodeType === Node.TEXT_NODE) {
            visibleTextFound = $.trim($(itm).text()).length > 0
            if (visibleTextFound)
                return false
        }
        if ($(itm).is(':visible'))
            visibleText(itm);
    })
}


function applyDoxTrace()
{
    ts = document.querySelector('meta[name="dox_trace_storage"]').content

    value = localStorage["doxTraceReadingModes_" + ts]
    if (value === "none")
    {
        $('.dox-trace-attribute-empty').show()
        $('.dox-trace-attribute-missing').show()
        $('.dox-trace-attribute').show()
    }
    if (value === "all")
    {
        $('.dox-trace-attribute-empty').hide()
        $('.dox-trace-attribute-missing').hide()
        $('.dox-trace-attribute').hide()
    }
    else if (value === "empty")
    {
        $('.dox-trace-attribute-empty').hide()
        $('.dox-trace-attribute-missing').show()
        $('.dox-trace-attribute').show()
    }
    else if (value === "missing")
    {
        $('.dox-trace-attribute-empty').hide()
        $('.dox-trace-attribute-missing').hide()
        $('.dox-trace-attribute').show()
    }

    $(".dox-trace-hide-if-empty td").each(function()
    {
        visibleTextFound = false
        visibleText(this)

        if (visibleTextFound)
        {
            $(this).parent().show();
        }
        else
        {
            $(this).parent().hide();
        }
    })
}

function dox_trace_config_changed(radiobutton)
{
    storage = document.querySelector('meta[name="dox_trace_storage"]')
    if (storage != null)
    {
        ts = storage.content

        localStorage["doxTraceReadingModes_" + ts] = radiobutton.value
        applyDoxTrace()
    }
}

$(function()
{
    storage = document.querySelector('meta[name="dox_trace_storage"]')
    if (storage != null)
    {
        ts = storage.content
        value = localStorage["doxTraceReadingModes_" + ts]

        $("#dox_trace_config_none").prop('checked', value === "none" || value === undefined);
        $("#dox_trace_config_all").prop('checked', value === "all");
        $("#dox_trace_config_empty").prop('checked', value === "empty");
        $("#dox_trace_config_missing").prop('checked', value === "missing");

        applyDoxTrace()

        $('.dox-trace-initially-hidden').css("visibility", "visible");
        $(".dox-trace-initially-hidden").addClass('dox-trace-remove-opacity');

        // uncomment to delete from storage for testing
        // localStorage.removeItem("doxTraceReadingModes_" + ts);
    }

    const modals = Array.from(document.querySelectorAll('[id^=traceabilityReportModal_]'));
    modals.forEach(function(modal) {
        postfix = modal.id.substring(23)

        const btn = document.getElementById("traceabilityReportButton" + postfix);
        const span = document.getElementById("traceabilityReportClose" + postfix);

        btn.onclick = function() { modal.style.display = "block"; }
        span.onclick = function() { modal.style.display = "none"; }

        window.addEventListener("click", function(event) {
            if (event.target === modal) { modal.style.display = "none"; }
        });
    })


});

