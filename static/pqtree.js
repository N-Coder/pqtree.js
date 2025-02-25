const randInt = (min, max) => Math.floor(Math.random() * (max - min + 1)) + min;
const range = (n) => [...Array(n).keys()];
const rowOf = (cell) => cell.closest("tr");

const cellOf = (cell) => cell.closest("td");

if (window.NodeList && !NodeList.prototype.filter) {
    NodeList.prototype.filter = Array.prototype.filter;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

let data = [];
let titles = [];

let dragSrcCell = null;
let dragType = null;

Module = {
    onRuntimeInitialized: function () {
        if (window.location.hash) {
            readURL();
        } else {
            const initialData = ["0010100", "0110100", "0111010"];
            data = [];
            for (let i = 0; i < initialData.length; i++) {
                data.push(initialData[i].split("").map(x => parseInt(x)));
            }
            buildTable();
            update();
        }
    }
};

function ensureTitles() {
    if (!titles) {
        titles = [];
    }
    if (!data || data.length < 1) {
        return;
    }
    while (titles.length < data[0].length) {
        let x = titles.length;
        let s = "";
        while (!s || x > 0) {
            s += String.fromCharCode("a".charCodeAt(0) + (x % 26));
            x = Math.floor(x / 26);
        }
        titles.push(s);
    }
}

function buildTable() {
    ensureTitles();
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
    let hrow = thead.insertRow();
    let cell = hrow.insertCell(); // title column
    for (let i = 0; i < data[0].length; i++) {
        let cell = hrow.insertCell();
        cell.classList.add('col-title');
        cell.innerHTML = titles[i];
        cell.addEventListener("dblclick", (e) => {
            const res = prompt("Rename column " + i + " '" + titles[i] + "'", titles[i]).trim();
            if (res.length > 0) {
                cell.innerHTML = titles[i] = res;
                update();
            }
        });
    }
    let addColCell = hrow.insertCell();
    addColCell.id = "add-col-cell";
    addColCell.innerHTML = '<a onclick="addColumn()">+</a>';
    // make data rows
    for (let i = 0; i < data.length; i++) {
        let row = tbody.insertRow();
        cell = row.insertCell();
        cell.innerHTML = i;
        cell.classList.add('row-title');
        for (let j = 0; j < data[0].length; j++) {
            cell = row.insertCell();
            let checkbox = document.createElement('input');
            checkbox.type = 'checkbox';
            checkbox.checked = data[i][j] === 1;
            checkbox.id = 'checkbox-' + i + '-' + j;
            checkbox.classList.add('input-checkbox');
            checkbox.addEventListener("change", update);
            cell.appendChild(checkbox);
        }
    }
    // make foot row
    let frow = tfoot.insertRow();
    let addRowCell = frow.insertCell();
    addRowCell.id = "add-row-cell";
    addRowCell.innerHTML = '<a onclick="addRow()">+</a>';


    // drag'n'drop cell callbacks
    function dragend(e) {
        //console.log("dragend", dragSrcCell, dragType, e.target, cellOf(e.target) ? cellOf(e.target).cellIndex : "?", rowOf(e.target) ? rowOf(e.target).rowIndex : "?")
        dragType = null;
        table.querySelectorAll('td').forEach(el =>
            el.classList.remove('dragged-col-source', 'dragged-col-target', 'dragged-col-target-before', 'dragged-col-target-after'));
        table.querySelectorAll('tr').forEach(el =>
            el.classList.remove('dragged-row-source', 'dragged-row-target', 'dragged-row-target-before', 'dragged-row-target-after'));
        table.classList.remove('table-dragging', 'table-dragging-row', 'table-dragging-col');
        addRowCell.classList.remove('del-btn-hover');
        addColCell.classList.remove('del-btn-hover');
    }

    function dragleave(e) {
        //console.log("dragleave", dragSrcCell, dragType, e.target, cellOf(e.target).cellIndex, rowOf(e.target).rowIndex)
        table.querySelectorAll("tr td:nth-child(" + (cellOf(e.target).cellIndex + 1) + ")").forEach(n =>
            n.classList.remove('dragged-col-target', 'dragged-col-target-before', 'dragged-col-target-after'));
        rowOf(e.target).classList.remove('dragged-row-target', 'dragged-row-target-before', 'dragged-row-target-after');
    }

    function dragenter(e) {
        //console.log("dragenter", dragSrcCell, dragType, e.target, cellOf(e.target).cellIndex, rowOf(e.target).rowIndex)
        if (dragType == "col") {
            const cellIndex = cellOf(e.target).cellIndex;
            for (let el of table.querySelectorAll("tr td:nth-child(" + (cellIndex + 1) + ")")) {
                el.classList.add('dragged-col-target');
                if (dragSrcCell.cellIndex == cellIndex || cellIndex == 0) {
                    el.classList.remove('dragged-col-target', 'dragged-col-target-before', 'dragged-col-target-after');
                } else if (dragSrcCell.cellIndex < cellIndex) {
                    el.classList.add('dragged-col-target-after');
                } else {
                    el.classList.add('dragged-col-target-before');
                }
            }
        } else if (dragType == "row") {
            let row = rowOf(e.target);
            row.classList.add('dragged-row-target');
            if (rowOf(dragSrcCell).rowIndex == row.rowIndex || row.rowIndex == 0) {
                row.classList.remove('dragged-row-target', 'dragged-row-target-before', 'dragged-row-target-after');
            } else if (rowOf(dragSrcCell).rowIndex < row.rowIndex) {
                row.classList.add('dragged-row-target-after');
            } else {
                row.classList.add('dragged-row-target-before');
            }
        } else return;
        e.preventDefault();
    }

    function drop(e) {
        //console.log("drop", dragSrcCell, dragType, e.target, cellOf(e.target).cellIndex, rowOf(e.target).rowIndex)
        if (dragType == "col") {
            const cellIndex = cellOf(e.target).cellIndex;
            if (dragSrcCell.cellIndex != cellIndex && cellIndex != 0) {
                const srci = dragSrcCell.cellIndex + 1;
                const tgti = cellIndex + 1;
                for (let row of table.querySelectorAll("thead tr, tbody tr")) {
                    const src = row.querySelector(":nth-child(" + srci + ")");
                    const tgt = row.querySelector(":nth-child(" + tgti + ")");
                    // console.log(row, src, tgt, dragSrcCell.cellIndex, cellIndex)
                    if (srci < tgti) {
                        tgt.insertAdjacentElement('afterend', src);
                    } else {
                        tgt.insertAdjacentElement('beforebegin', src);
                    }
                }
            }
            table.querySelectorAll('.dragged-col-target').forEach(el =>
                el.classList.remove('dragged-col-target', 'dragged-col-target-before', 'dragged-col-target-after'));
        } else if (dragType == "row") {
            const tgtRow = rowOf(e.target);
            const srcRow = rowOf(dragSrcCell);
            const srcRowIdx = srcRow.rowIndex;
            if (srcRowIdx != tgtRow.rowIndex && tgtRow.rowIndex != 0) {
                if (srcRowIdx < tgtRow.rowIndex) {
                    tgtRow.insertAdjacentElement('afterend', srcRow);
                } else {
                    tgtRow.insertAdjacentElement('beforebegin', srcRow);
                }
            }
            table.querySelectorAll('.dragged-row-target').forEach(el =>
                el.classList.remove('dragged-row-target', 'dragged-row-target-before', 'dragged-row-target-after'));
        } else return;
        table.classList.remove('table-dragging', 'table-dragging-row', 'table-dragging-col');
        e.preventDefault();
        update();
    }

    for (let cell of table.querySelectorAll("td.col-title, td.row-title, td:has(input.input-checkbox)")) {
        cell.addEventListener('dragend', dragend);
        cell.addEventListener('dragleave', dragleave);
        cell.addEventListener('dragover', dragenter);
        cell.addEventListener('dragenter', dragenter);
        cell.addEventListener('drop', drop);
    }

    addRowCell.addEventListener('dragleave', (e) => {
        //console.log("dragleaveR", dragSrcCell, dragType, e.target, cellOf(e.target).cellIndex, rowOf(e.target).rowIndex)
        addRowCell.classList.remove('del-btn-hover')
    });
    addRowCell.addEventListener('dragover', (e) => {
        //console.log("dragoverR", dragSrcCell, dragType, e.target, cellOf(e.target).cellIndex, rowOf(e.target).rowIndex)
        if (dragType == "row" && data.length > 1) {
            addRowCell.classList.add('del-btn-hover')
            e.preventDefault();
        }
    });
    addRowCell.addEventListener('dragenter', (e) => {
        //console.log("dragenterR", dragSrcCell, dragType, e.target, cellOf(e.target).cellIndex, rowOf(e.target).rowIndex)
        if (dragType == "row" && data.length > 1) {
            addRowCell.classList.add('del-btn-hover')
            e.preventDefault();
        }
    });
    addRowCell.addEventListener('drop', (e) => {
        //console.log("dropR", dragSrcCell, dragType, e.target, cellOf(e.target).cellIndex, rowOf(e.target).rowIndex)
        if (dragType == "row" && data.length > 1) {
            rowOf(dragSrcCell).remove();
            e.preventDefault();
            update();
        }
        addRowCell.classList.remove('del-btn-hover')
    });

    addColCell.addEventListener('dragleave', (e) => {
        //console.log("dragleaveC", dragSrcCell, dragType, e.target, cellOf(e.target).cellIndex, rowOf(e.target).rowIndex)
        addColCell.classList.remove('del-btn-hover')
    });
    addColCell.addEventListener('dragover', (e) => {
        //console.log("dragoverC", dragSrcCell, dragType, e.target, cellOf(e.target).cellIndex, rowOf(e.target).rowIndex)
        if (dragType == "col" && data[0].length > 2) {
            addColCell.classList.add('del-btn-hover')
            e.preventDefault();
        }
    });
    addColCell.addEventListener('dragenter', (e) => {
        //console.log("dragenterC", dragSrcCell, dragType, e.target, cellOf(e.target).cellIndex, rowOf(e.target).rowIndex)
        if (dragType == "col" && data[0].length > 2) {
            addColCell.classList.add('del-btn-hover')
            e.preventDefault();
        }
    });
    addColCell.addEventListener('drop', (e) => {
        //console.log("dropC", dragSrcCell, dragType, e.target, cellOf(e.target).cellIndex, rowOf(e.target).rowIndex)
        if (dragType == "col" && data[0].length > 2) {
            table.querySelectorAll("tr td:nth-child(" + (cellOf(dragSrcCell).cellIndex + 1) + ")").forEach(n => n.remove());
            e.preventDefault();
            update();
        }
        addColCell.classList.remove('del-btn-hover')
    });

    // enable col drag-rearrangement
    for (let col of thead.querySelectorAll("td.col-title")) {
        col.draggable = true;
        col.addEventListener('dragstart', e => {
            dragType = "col";
            dragSrcCell = col;
            table.classList.add('table-dragging', 'table-dragging-col');
            e.dataTransfer.effectAllowed = 'move';

            const dragGhost = table.cloneNode(true);
            dragGhost.querySelectorAll("tr td:nth-child(-n+" + col.cellIndex + ")").forEach(n => n.remove());
            dragGhost.querySelectorAll("tr td:nth-child(n+2)").forEach(n => n.remove());
            e.dataTransfer.setData('text/html', dragGhost.outerHTML);
            const dragGhostWrapper = document.querySelector("#drag-ghost");
            dragGhostWrapper.innerHTML = "";
            dragGhostWrapper.appendChild(dragGhost);
            e.dataTransfer.setDragImage(dragGhost, 0, 0);

            table.querySelectorAll("tr td:nth-child(" + (col.cellIndex + 1) + ")").forEach(
                n => n.classList.add('dragged-col-source'));
        });
    }

    // enable row drag-rearrangement
    for (let row of tbody.querySelectorAll("tr td.row-title")) {
        row.draggable = true;
        row.addEventListener('dragstart', e => {
            dragType = "row";
            dragSrcCell = row;
            table.classList.add('table-dragging', 'table-dragging-row');
            e.dataTransfer.effectAllowed = 'move';

            const dragGhost = table.cloneNode(true);
            dragGhost.querySelectorAll("tr:nth-child(-n+" + (rowOf(row).rowIndex - 1) + ")").forEach(n => n.remove());
            dragGhost.querySelectorAll("tr:nth-child(n+2), thead tr, tfoot tr").forEach(n => n.remove());
            e.dataTransfer.setData('text/html', dragGhost.outerHTML);
            const dragGhostWrapper = document.querySelector("#drag-ghost");
            dragGhostWrapper.innerHTML = "";
            dragGhostWrapper.appendChild(dragGhost);
            e.dataTransfer.setDragImage(dragGhost, 0, 0);

            rowOf(row).classList.add('dragged-row-source');
        });
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
        for (let j = 1; j < row.cells.length; j++) { // skip title col
            let cell = row.cells[j];
            let checkbox = cell.getElementsByTagName('input')[0];
            line.push(checkbox.checked ? 1 : 0);
        }
        data.push(line);
    }
    titles = Array.from(table.querySelectorAll("thead td.col-title").values().map(n => n.innerHTML));
    ensureTitles();

    writeURL();

    const is_circular = document.getElementById("toggle-cyclic").checked;
    const failed_restr = Module.setRestrictions(data.map(line => line.join("")).join("\n"), is_circular, titles.join("|"));
    // document.getElementById("display").innerHTML = res + " " + Module.getLeafOrder();

    if (is_circular) {
        document.getElementById("export-label").innerHTML = "Export PC-tree figure";
    } else {
        document.getElementById("export-label").innerHTML = "Export PQ-tree figure";
    }
    const svg_cont = document.getElementById("svg-container");
    const tikz_cont = document.getElementById("tikz-container");

    const params = Module.getDrawingParams();
    params.levelHeight = document.getElementById("cl-levelheight").value;
    params.nodeSize = document.getElementById("cl-nodesize").value;
    params.nodePadding = document.getElementById("cl-nodepadding").value;
    params.radius = document.getElementById("cl-radius").value;
    document.querySelectorAll("#config-layout .cl-pq").forEach((e) => e.style.display = is_circular ? "none" : "");
    document.querySelectorAll("#config-layout .cl-pc").forEach((e) => e.style.display = is_circular ? "" : "none");

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

let block_url_updates = false;

function writeURL() {
    const hash = computeHash();
    if (hash != window.location.hash && !block_url_updates) {
        // console.log("wrote hash", hash)
        window.history.pushState(hash, "", window.location.origin + window.location.pathname + hash);
    }
}

function readURL() {
    if (window.location.hash && window.location.hash != computeHash()) {
        const hash = window.location.hash.substring(1);
        // console.log("read hash", hash)
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

        function finalizeTut() {
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
            block_url_updates = false;
        }

        block_url_updates = true;
        if ("tut" in vars) {
            if (!tg.isVisible) {
                tg.activeStep = vars["tut"] * 1
                tg.start().finally(finalizeTut);
            } else {
                tg.visitStep(vars["tut"] * 1).finally(finalizeTut);
            }
        } else {
            if (tg.isVisible) {
                setTimeout(() => tg.exit().finally(finalizeTut), 100);
            } else {
                finalizeTut();
            }
        }
    }
}

window.addEventListener('hashchange', readURL)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

document.addEventListener('paste', (event) => {
    event.preventDefault();
    const paste = (event.clipboardData || window.clipboardData).getData('text').trim();
    const lines = paste.split('\n').map(s => s.trim());
    if (!lines || lines.length < 1) {
        return;
    }
    let new_titles, new_data;
    let start = 0;
    if (!lines[0].match(/^[01]+$/)) {
        new_titles = lines[0].split("|");
        start = 1;
    }
    new_data = [];
    for (let i = start; i < lines.length; i++) {
        new_data.push(lines[i].split("").map(x => parseInt(x)));
        if (new_data.length > 1 && new_data[0].length != new_data.at(-1).length) {
            console.error("Rejecting line with mismatching row lengths", lines[i]);
            new_data.pop();
        }
    }
    if (new_data && new_data.length >= 1 && new_data[0].length > 1) {
        titles = new_titles;
        data = new_data
        buildTable();
        update();
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
        titles = lines[0].split("|");
        data = [];
        for (let i = 1; i < lines.length; i++) {
            const l = lines[i].trim();
            if (!l) continue;
            data.push(l.split("").map(x => parseInt(x)));
        }
        buildTable();
        update();
    }
}

function randomMatrix() {
    const is_circular = document.getElementById("toggle-cyclic").checked;
    let n, rows;
    if (window.innerWidth <= 600) {
        n = randInt(5, 9);
        rows = randInt(4, 6);
    } else {
        n = randInt(5, 12);
        rows = randInt(3, 8);
    }
    titles = [];
    data = [];
    for (let i = 0; i < rows; i++) {
        let left = randInt(0, n - 2);
        let right = randInt(left + 1, n - 1);
        let toggle = !is_circular || randInt(1, 3) !== 1;
        data.push(range(n).map((i) => ((left <= i && i <= right) === toggle) ? 1 : 0));
    }
    buildTable();
    update();
}

function addColumn() {
    for (let i = 0; i < data.length; i++) {
        data[i].push(0);
    }
    buildTable();
    update();
}

function addRow() {
    data.push(Array(data[0].length).fill(0));
    buildTable();
    update();
}

function reorderMatrix() {
    const order = Module.getLeafOrder().split(" ");
    // console.log(order);
    let new_data = [];
    for (let i = 0; i < data.length; i++) {
        new_data.push(range(data[0].length).map(j => data[i][order[j]]));
    }
    data = new_data;
    if (data.length > 0) {
        let new_title = [];
        for (let i = 0; i < data[0].length; i++) {
            new_title.push(titles[order[i]]);
        }
        // console.log(order, titles, new_title)
        titles = new_title;
    }
    buildTable();
    update();
}

function copyMatrix() {
    let text = titles.join("|") + "\n";
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
    const selected = e.target.dataset.leaves.split(" ");
    const idcs = document.querySelectorAll("#input-table thead td.col-title")
        .filter((e) => selected.includes(e.innerHTML.trim()))
        .map((e) => e.cellIndex)
    // console.log(selected, idcs)
    const tbody = document.querySelector("#input-table tbody")
    for (let idx of idcs) {
        for (let checkbox of tbody.querySelectorAll("tr td:nth-child(" + (idx + 1) + ") input.input-checkbox")) {
            checkbox.classList.add("highlight");
        }
    }
}

function nodeMouseOut(e) {
    for (let checkbox of document.querySelectorAll("input.input-checkbox.highlight")) {
        checkbox.classList.remove("highlight");
    }
}
