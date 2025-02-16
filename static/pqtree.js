const randInt = (min, max) => Math.floor(Math.random() * (max - min + 1)) + min;
const range = (n) => [...Array(n).keys()];

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

var n;
var data = [];
var titles = [];

var Module = {
    onRuntimeInitialized: function () {
        if (window.location.hash) {
            readURL();
        } else {
            const initialData = ["001010", "011010", "011101"];
            data = [];
            for (let i = 0; i < initialData.length; i++) {
                data.push(initialData[i].split("").map(x => parseInt(x)));
            }
            buildTable();
            update();
        }
    }
};

function buildTable() {
    const n = data[0].length;
    if (!titles) {
        titles = [];
    }
    while (titles.length < n) {
        titles.push(titles.length + "");
    }
    let table = document.createElement('table');
    let thead = document.createElement('thead');
    let tbody = document.createElement('tbody');
    let tfoot = document.createElement('tfoot');
    table.appendChild(thead);
    table.appendChild(tbody);
    table.appendChild(tfoot);
    document.getElementById('input-table-wrapper').replaceChild(table, document.getElementById('input-table'));
    table.id = 'input-table';
    // make header
    let row = thead.insertRow();
    for (let i = 0; i < n; i++) {
        let cell = row.insertCell();
        cell.innerHTML = titles[i];
    }
    let addCell = row.insertCell();
    addCell.innerHTML = '<a onclick="addColumn()">+</a>';
    // make data rows
    for (let i = 0; i < data.length; i++) {
        let row = tbody.insertRow();
        for (let j = 0; j < n; j++) {
            cell = row.insertCell();
            let checkbox = document.createElement('input');
            checkbox.type = 'checkbox';
            checkbox.checked = data[i][j] === 1;
            checkbox.id = 'checkbox-' + i + '-' + j;
            checkbox.classList.add('input-checkbox');
            cell.appendChild(checkbox);
        }
    }
    // make foot row
    row = tfoot.insertRow();
    cell = row.insertCell();
    cell.id = "add-row-cell";
    cell.innerHTML = '<a onclick="addRow()">+</a>';
    // add event listeners to all checkboxes
    for (var checkbox of tbody.getElementsByTagName("input")) {
        checkbox.addEventListener("click", update);
    }
}

function update(e) {
    // update data from checkboxes
    data = [];
    let table = document.getElementById('input-table');
    let tbody = table.getElementsByTagName('tbody')[0];
    for (let i = 0; i < tbody.rows.length; i++) {
        let row = tbody.rows[i];
        let line = [];
        for (let j = 0; j < row.cells.length; j++) {
            let cell = row.cells[j];
            let checkbox = cell.getElementsByTagName('input')[0];
            line.push((checkbox.checked ? 1 : 0));
        }
        data.push(line);
    }
    n = data[0].length;
    if (!titles) {
        titles = [];
    }
    while (titles.length < n) {
        titles.push(titles.length + "");
    }
    // console.log(n, data);

    writeURL();

    const is_circular = document.getElementById("toggle-cyclic").checked;
    const failed_restr = Module.setRestrictions(data.map(line => line.join("")).join("\n"), is_circular, titles.join("|"));
    // document.getElementById("display").innerHTML = res + " " + Module.getLeafOrder();

    const svg_cont = document.getElementById("svg-container");
    const tikz_cont = document.getElementById("tikz-container");
    svg_cont.innerHTML = Module.drawSVG(is_circular);
    tikz_cont.innerHTML = Module.drawTikz(is_circular);
    for (let node of svg_cont.childNodes[0].childNodes) {
        if (node.tagName === "rect" || node.tagName === "circle" || node.tagName === "polygon") {
            node.addEventListener("mouseover", nodeMouseOver);
            node.addEventListener("mouseout", nodeMouseOut);
        }
    }

    const svg_down = document.getElementById("svg-download");
    const ipe_down = document.getElementById("ipe-download");
    svg_down.innerHTML = "Download SVG";
    svg_down.href = URL.createObjectURL(new Blob([svg_cont.innerHTML], {type: "image/svg+xml;charset=utf-8"}));
    svg_down.download = is_circular ? "PCtree.svg" : "PQtree.svg";
    ipe_down.innerHTML = "Download IPE";
    ipe_down.href = URL.createObjectURL(new Blob([Module.drawIPE(is_circular)], {type: "text/xml;charset=utf-8"}));
    ipe_down.download = is_circular ? "PCtree.ipe" : "PQtree.ipe";

    document.getElementById("input-serialize").value = Module.serializeTree(is_circular);

    document.querySelectorAll("#input-table tr").forEach(tr => tr.classList.remove("error"));
    if (failed_restr < 0) {
        var num = Module.getOrderCount();
        document.getElementById("numberOfEncodedOrderings").innerHTML = num;
        if (num < 250) {
            document.getElementById("allOrderings").innerHTML = Module.getAllOrders();
        } else {
            document.getElementById("allOrderings").innerHTML = "Too many orderings to display";
        }
        document.getElementById("reorder-button").disabled = false;
    } else {
        document.getElementById("numberOfEncodedOrderings").innerHTML = "0";
        document.getElementById("allOrderings").innerHTML = "No valid order";
        document.getElementById("reorder-button").disabled = true;
        document.querySelectorAll("#input-table tbody tr:nth-child(" + (failed_restr + 1) + ")")
            .forEach(tr => tr.classList.add("error"));
    }
    document.getElementById("serialize-error").innerHTML = "";

    if (tg.isVisible) {
        setTimeout(function () {
            tg.updatePositions();
        }, 100);
    }
}

document.getElementById("toggle-cyclic").addEventListener("click", update);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

function computeHash() {
    let vars = {
        "mat": data.map(row => row.join("")).join("|"),
        "circ": document.getElementById("toggle-cyclic").checked * 1,
    }
    if (tg.isVisible) {
        vars["tut"] = tg.activeStep;
    }
    if (data && data.length > 0) {
        const n = data[0].length;
        let tit_ref = [];
        for (let i = 0; i < n; i++) {
            tit_ref.push(i + "");
        }
        const tit_act = titles.join("|");
        if (tit_act != tit_ref.join("|")) {
            vars["col"] = tit_act;
        }
    }
    return "#" + Object.entries(vars).map(t => t.join("=")).join("&");
}

function writeURL() {
    const hash = computeHash();
    if (hash != window.location.hash) {
        window.history.pushState(hash, "", window.location.origin + window.location.pathname + hash);
    }
}

function readURL() {
    if (window.location.hash && window.location.hash != computeHash()) {
        const hash = window.location.hash.substring(1);
        const parts = hash.split('&');
        let vars = {};
        for (let i = 0; i < parts.length; i++) {
            const pair = parts[i].split('=');
            if (pair.length == 1 && parts.length == 1) {
                vars["mat"] = pair[0];
            } else {
                vars[decodeURIComponent(pair[0])] = decodeURIComponent(pair[1]);
            }
        }

        if ("mat" in vars) {
            const mat_rows = vars["mat"].split("|");
            data = [];
            for (let i = 0; i < mat_rows.length; i++) {
                data.push(mat_rows[i].split("").map(x => parseInt(x)));
            }
        }
        if ("col" in vars) {
            titles = vars["col"].split("|");
        } else {
            titles = [];
        }

        if ("circ" in vars) {
            document.getElementById("toggle-cyclic").checked = vars["circ"] > 0
        }

        buildTable();
        update();

        if ("tut" in vars) {
            if (!tg.isVisible) {
                tg.activeStep = vars["tut"] * 1
                tg.start()
            } else {
                tg.visitStep(vars["tut"] * 1)
            }
        } else {
            tg.exit()
        }
    }
}

window.addEventListener('hashchange', readURL)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

document.addEventListener('paste', (event) => {
    let paste = (event.clipboardData || window.clipboardData).getData('text');
    // throw away all leading/trailing space and all characters except 0, 1, and \n
    paste = paste.replace(/^\s+|\s+$/g, '');
    if (paste.match(/^[01\n]+$/)) {
        var lines = paste.split('\n');
        // check that all lines have the same length
        if (lines.every(line => line.length === lines[0].length)) {
            for (let i = 0; i < lines.length; i++) {
                data[i] = lines[i].split("").map(x => parseInt(x));
            }
            buildTable();
            update();
        } else {
            console.error("Could not parse clipboard data with mismatching row lengths", lines);
        }
        event.preventDefault();
    }
});

function deserialize() {
    const is_circular = document.getElementById("toggle-cyclic").checked;
    const spec = document.getElementById("input-serialize").value;
    const matrix = Module.treeSpecToMatrix(spec, is_circular);
    if (matrix[0] === "!") {
        document.getElementById("serialize-error").innerHTML = matrix.substring(1)
    } else {
        document.getElementById("serialize-error").innerHTML = "";
        const lines = matrix.split('\n');
        for (let i = 0; i < lines.length; i++) {
            data[i] = lines[i].split("").map(x => parseInt(x));
        }
        buildTable();
        update();
    }
}

function randomMatrix() {
    const is_circular = document.getElementById("toggle-cyclic").checked;
    if (window.innerWidth <= 600) {
        n = randInt(5, 9);
        var rows = randInt(4, 6);
    } else {
        n = randInt(5, 12);
        var rows = randInt(3, 8);
    }
    titles = [];
    data = [];
    for (let i = 0; i < rows; i++) {
        var left = randInt(0, n - 2);
        var right = randInt(left + 1, n - 1);
        var toggle = !is_circular || randInt(1, 3) !== 1;
        data.push(range(n).map((i) => ((left <= i && i <= right) === toggle) ? 1 : 0));
    }
    buildTable();
    update();
}

function addColumn() {
    n += 1;
    for (let i = 0; i < data.length; i++) {
        data[i].push(0);
    }
    buildTable();
    update();
}

function addRow() {
    data.push(Array(n).fill(0));
    buildTable();
    update();
}

function reorderMatrix() {
    const order = Module.getLeafOrder().split(" ");
    // console.log(order);
    let new_data = [];
    for (let i = 0; i < data.length; i++) {
        new_data.push(range(n).map(j => data[i][order[j]]));
    }
    data = new_data;
    if (data.length > 0) {
        let new_title = [];
        for (let i = 0; i < data[0].length; i++) {
            new_title.push(titles[order[i]]);
        }
        console.log(order, titles, new_title)
        titles = new_title;
    }
    buildTable();
    update();
}

function copyMatrix() {
    let text = "";
    for (let i = 0; i < data.length; i++) {
        text += data[i].join("") + "\n";
    }
    navigator.clipboard.writeText(text).then(() => {
        console.log('Async: Copying to clipboard was successful!');
    }, (err) => {
        console.error('Async: Could not copy text: ', err);
    });
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

function nodeMouseOver(e) {
    if (!e.target.dataset.hasOwnProperty("leaves")) {
        return;
    }
    var leaves = e.target.dataset.leaves.split(" ");
    for (var checkbox of document.getElementsByClassName("input-checkbox")) {
        if (leaves.includes(checkbox.id.split("-")[2])) {
            checkbox.classList.add("highlight");
        }
    }
}

function nodeMouseOut(e) {
    for (var checkbox of document.getElementsByClassName("input-checkbox")) {
        checkbox.classList.remove("highlight");
    }
}
