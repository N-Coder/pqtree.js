const randInt = (min, max) => Math.floor(Math.random() * (max - min + 1)) + min;
const range = (n) => [...Array(n).keys()];

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

var n;
var data;

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
    n = data[0].length;
    let table = document.createElement('table');
    let thead = document.createElement('thead');
    let tbody = document.createElement('tbody');
    let tfoot = document.createElement('tfoot');
    table.appendChild(thead);
    table.appendChild(tbody);
    table.appendChild(tfoot);
    document.getElementById('input-table-container').replaceChild(table, document.getElementById('input-table'));
    table.id = 'input-table';
    // make header
    let row = thead.insertRow();
    for (let i = 0; i < n; i++) {
        let cell = row.insertCell();
        cell.innerHTML = i;
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
    // console.log(n, data);

    writeURL();

    const is_circular = true;
    const res = Module.setRestrictions(data.map(line => line.join("")).join("\n"), is_circular);
    // document.getElementById("display").innerHTML = res + " " + Module.getLeafOrder();

    if (res) {
        document.getElementById("svg-container").innerHTML = Module.drawSVG(is_circular);
        var num = Module.getOrderCount();
        document.getElementById("numberOfEncodedOrderings").innerHTML = num;
        if (num < 250) {
            document.getElementById("allOrderings").innerHTML = Module.getAllOrders();
        } else {
            document.getElementById("allOrderings").innerHTML = "Too many orderings to display";
        }
    } else {
        document.getElementById("svg-container").innerHTML = "";
        document.getElementById("numberOfEncodedOrderings").innerHTML = "0";
        document.getElementById("allOrderings").innerHTML = "No valid order";
    }
    document.getElementById("reorder-button").disabled = !res;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

function writeURL() {
    window.history.pushState(data, "", window.location.origin + window.location.pathname + "#" + data.map(row => row.join("")).join("&"));
}

window.addEventListener('popstate', readURL);

function readURL() {
    if (window.location.hash) {
        var params = window.location.hash.substr(1).split("&");
        data = [];
        for (var i = 0; i < params.length; i++) {
            var row = params[i].split("").map(x => parseInt(x));
            data.push(row);
        }
        buildTable();
        update();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

document.addEventListener('paste', (event) => {
    let paste = (event.clipboardData || window.clipboardData).getData('text');
    // throw away all leading/trailing space and all characters except 0, 1, and \n
    paste = paste.replace(/^\s+|\s+$/g, '').replace(/[^01\n]/g, '');
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
    }
    event.preventDefault();
});

function randomMatrix() {
    if (window.innerWidth <= 600) {
        n = randInt(5, 9);
        var rows = randInt(4, 6);
    } else {
        n = randInt(5, 12);
        var rows = randInt(3, 8);
    }
    data = [];
    for (let i = 0; i < rows; i++) {
        var left = randInt(0, n - 2);
        var right = randInt(left + 1, n - 1);
        data.push(range(n).map((i) => left <= i && i <= right ? 1 : 0));
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
    var order = Module.getLeafOrder().split(" ");
    // console.log(order);
    var new_data = [];
    for (let i = 0; i < data.length; i++) {
        new_data.push(range(n).map(j => data[i][order[j]]));
    }
    data = new_data;
    buildTable();
    update();
}

function copyMatrix() {
    var text = "";
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
