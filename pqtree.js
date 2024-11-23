var n;
var data;
var pqT;

const range = (n) => [...Array(n).keys()];

var process;
PQTreeModule().then(function (Module) {
    process = Module.cwrap('Process', 'string', ['string']);

    if (window.location.hash) {
        readURL();
        return;
    }

    const initialData = ["001010", "011010", "011101"];
    data = [];
    for (let i = 0; i < initialData.length; i++) {
        data.push(initialData[i].split("").map(x => parseInt(x)));
    }
    buildTable();
    update();
});

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

document.addEventListener('paste', (event) => {
    let paste = (event.clipboardData || window.clipboardData).getData('text');
    // throw away all characters except 0, 1, and \n
    paste = paste.replace(/[^01\n]/g, '');
    if (paste.match(/^[01\n]+$/)) {
        var lines = paste.split('\n');
        // check that all lines have the same length
        if (lines.every(line => line.length === lines[0].length)) {
            for (let i = 0; i < lines.length; i++) {
                data[i] = lines[i].split("").map(x => parseInt(x));
            }
            buildTable();
            update();
        }
    }
    event.preventDefault();
});

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

function randInt(min, max) {
    return Math.floor(Math.random() * (max - min + 1)) + min;
}

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

function leafOrder(node) {
    if (node.type == 'leaf') {
        return [node.value];
    } else {
        var order = [];
        for (let i = 0; i < node.children.length; i++) {
            order = order.concat(leafOrder(node.children[i]));
        }
        return order;
    }
}

function reorderMatrix() {
    if (pqT == null) {
        return;
    }
    var order = leafOrder(pqT);
    console.log(order);
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

function isdigit(str) {
    return /^\d+$/.test(str);
}
if (!Array.prototype.last) {
    Array.prototype.last = function () {
        return this[this.length - 1];
    };
};

function pqTree(output) {
    // parse output
    if (output == "Impossible") {
        return null;
    }
    var stack = [{ "type": "root", "children": [] }];
    var reading_number = false;
    for (var i = 0; i < output.length; i++) {
        var char = output[i];
        if (reading_number) {
            if (isdigit(char)) {
                number = number + char;
                continue;
            } else {
                stack.last().children.push({ "type": "leaf", "value": parseInt(number) });
                reading_number = false;
            }
        }
        if (char == '(') {
            stack.push({ "type": "pnode", "children": [] });
        } else if (char == ')') {
            var node = stack.pop();
            stack.last().children.push(node);
        } else if (char == '[') {
            stack.push({ "type": "qnode", "children": [] });
        } else if (char == ']') {
            var node = stack.pop();
            stack.last().children.push(node);
        } else if (char == ' ') {
            continue;
        } else if (isdigit(char)) {
            reading_number = true;
            number = "";
            number = number + char;
        } else {
            throw "Unknown char {}".format(char);
        }
    }
    return stack[0]["children"][0];
}

function writeURL() {
    window.history.pushState(data, "", window.location.origin + window.location.pathname + "#" + data.map(row => row.join("")).join("&"));
}

// read URL
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

var oldMessage = "";
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

    message = n + ";" + data.map(line => line.join("")).join(";");
    if (message == oldMessage) {
        return;
    }
    writeURL();
    output = process(message);
    oldMessage = message;
    // document.getElementById("display").innerHTML = output;
    pqT = pqTree(output);

    drawPQTree(pqT);

    var num = numberOfEncodedOrderings(pqT);
    document.getElementById("numberOfEncodedOrderings").innerHTML = num;
    if (num < 250) {
        var orderings = allOrderings(pqT);
        document.getElementById("allOrderings").innerHTML = orderings.join("\n");
    } else {
        document.getElementById("allOrderings").innerHTML = "Too many orderings to display";
    }

    // de/activate reorder button
    var reorderButton = document.getElementById("reorder-button");
    if (pqT == null) {
        reorderButton.disabled = true;
    } else {
        reorderButton.disabled = false;
    }
}

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

function addSvgNode(base, n, v) {
    node = { type: n };
    for (var p in v) {
        if (p == "text") {
            node.textContent = v[p];
        } else {
            node[p.replace(/[A-Z]/g, function (m, p, o, s) { return "-" + m.toLowerCase(); })] = v[p];
        }
    }
    base.push(node);
    return node;
}

function nodeToSvg(node) {
    nx = document.createElementNS("http://www.w3.org/2000/svg", node.type);
    for (var p in node) {
        if (p == "type") {
            continue;
        } else if (p == "textContent") {
            nx.textContent = node[p];
        } else {
            nx.setAttributeNS(null, p, node[p]);
        }
    }
    if (node.type == "rect" || node.type == "circle" || node.type == "polygon") {
        nx.addEventListener("mouseover", nodeMouseOver);
        nx.addEventListener("mouseout", nodeMouseOut);
    }
    return nx;
}

function nodeToTikZ(node) {
    function scale(x) {
        return (x * 0.02);//.toPrecision(2);
    }
    if (node.type == "circle") {
        return "\\draw[pnode] (" + scale(node.cx) + "," + scale(-node.cy) + ") circle [radius=" + scale(node.r) + "];";
    } else if (node.type == "rect") {
        return "\\draw[qnode] (" + scale(node.x) + "," + scale(-node.y) + ") rectangle (" + (scale(node.x + node.width)) + "," + (scale(-node.y - node.height)) + ");";
    } else if (node.type == "text") {
        return "\\node at (" + scale(node.x) + "," + scale(-node.y) + ") {" + node.textContent + "};";
    } else if (node.type == "line") {
        return "\\draw (" + scale(node.x1) + "," + scale(-node.y1) + ") -- (" + scale(node.x2) + "," + scale(-node.y2) + ");";
    } else if (node.type == "polygon") {
        var s = "\\draw[leaf]";
        var points = node.points.split(" ");
        for (var point of points) {
            var xy = point.split(",");
            s = s + " (" + scale(xy[0]) + "," + scale(-xy[1]) + ") --";
        }
        s = s + "cycle;";
        return s;
    } else {
        throw "Unknown node type " + node.type;
    }
}

function numLeaves(node) {
    // count number of leaves that descend from node
    if (node.type == "leaf") {
        return 1;
    } else if (node.type == "pnode" || node.type == "qnode") {
        var sum = 0;
        for (var i = 0; i < node.children.length; i++) {
            sum += numLeaves(node.children[i]);
        }
        return sum;
    } else {
        throw "Unknown node type " + node.type;
    }
}

function getLeaves(node) {
    // count number of leaves that descend from node
    if (node.type == "leaf") {
        return node.value;
    } else if (node.type == "pnode" || node.type == "qnode") {
        var leaves = [];
        for (var i = 0; i < node.children.length; i++) {
            leaves = leaves.concat(getLeaves(node.children[i]));
        }
        return leaves.join(" ");
    } else {
        throw "Unknown node type " + node.type;
    }
}

function treeHeight(node) {
    if (node.type == "leaf") {
        return 1;
    } else if (node.type == "pnode" || node.type == "qnode") {
        var max = 0;
        for (var i = 0; i < node.children.length; i++) {
            var h = treeHeight(node.children[i]);
            if (h > max) {
                max = h;
            }
        }
        return max + 1;
    } else {
        throw "Unknown node type " + node.type;
    }
}

const leafWidth = 70;
const levelHeight = 80;

function buildPQNode(nodeList, node, cx, cy, incomingLine = null) {
    if (node.type == "leaf") {
        const ratio = 0.866; // equilateral triangle
        const sideLength = 40;
        addSvgNode(nodeList, 'polygon', {
            points: `${cx},${cy} ${cx - sideLength / 2},${cy + sideLength * ratio} ${cx + sideLength / 2},${cy + sideLength * ratio}`,
            fill: '#ececec',
            stroke: 'black',
            dataLeaves: getLeaves(node)
        });
        addSvgNode(nodeList, 'text', { x: cx, y: cy + 0.69 * sideLength * ratio, textAnchor: 'middle', dominantBaseline: 'middle', text: node.value });
        yValuesWhileRendering.push(cy + sideLength * ratio);
    } else if (node.type == "pnode") {
        var myWidth = numLeaves(node) * leafWidth;
        var left = cx - myWidth / 2;
        for (var child of node.children) {
            var childWidth = numLeaves(child) * leafWidth;
            var childCX = left + childWidth / 2;
            if (child.type == "leaf") {
                var childCY = cy + 0.5 * levelHeight;
            } else {
                var childCY = cy + levelHeight;
            }
            var line = addSvgNode(nodeList, 'line', { x1: cx, y1: cy, x2: childCX, y2: childCY, stroke: 'black' });
            buildPQNode(nodeList, child, childCX, childCY, line);
            left += childWidth;
        }
        addSvgNode(nodeList, 'circle', { cx: cx, cy: cy, r: 15, fill: '#ececec', stroke: 'black', dataLeaves: getLeaves(node) });
        addSvgNode(nodeList, 'text', { x: cx + 0.4, y: cy + 1, textAnchor: 'middle', dominantBaseline: 'middle', text: "P" });
    } else if (node.type == "qnode") {
        var myWidth = numLeaves(node) * leafWidth;
        var left = cx - myWidth / 2;
        var childCXs = [];
        for (var child of node.children) {
            var childWidth = numLeaves(child) * leafWidth;
            var childCX = left + childWidth / 2;
            childCXs.push(childCX);
            if (child.type == "leaf") {
                var childCY = cy + 0.5 * levelHeight;
            } else {
                var childCY = cy + levelHeight;
            }
            var line = addSvgNode(nodeList, 'line', { x1: childCX, y1: cy, x2: childCX, y2: childCY, stroke: 'black' });
            buildPQNode(nodeList, child, childCX, childCY, line);
            left += childWidth;
        }
        var myWidth = childCXs.last() - childCXs[0];
        var buffer = 0.2 * leafWidth;
        var myHeight = 0.3 * levelHeight;
        addSvgNode(nodeList, 'rect', {
            x: childCXs[0] - buffer,
            y: cy - myHeight / 2,
            width: myWidth + 2 * buffer,
            height: myHeight,
            fill: '#ececec',
            stroke: 'black',
            dataLeaves: getLeaves(node)
        });
        var center = childCXs[0] + myWidth / 2;
        addSvgNode(nodeList, 'text', { x: center, y: cy + 1, textAnchor: 'middle', dominantBaseline: 'middle', text: "Q" });
        if (incomingLine != null) {
            incomingLine.x2 = center;
        }
    } else {
        throw "Unknown node type " + node.type;
    }
}

var yValuesWhileRendering = [];
function drawPQTree(tree) {
    var oldSvg = document.getElementById("treeSVG");
    var svg = document.createElementNS("http://www.w3.org/2000/svg", "svg");
    oldSvg.parentNode.replaceChild(svg, oldSvg); // clear svg
    svg.id = "treeSVG";
    svg.setAttributeNS("http://www.w3.org/2000/xmlns/", "xmlns", "http://www.w3.org/2000/svg");
    svg.addEventListener("touchstart", nodeMouseOut);
    var downloadLink = document.getElementById("svgDownload");
    downloadLink.innerHTML = "";

    if (tree == null) {
        return;
    }

    yValuesWhileRendering = [];

    var overallWidth = numLeaves(tree) * leafWidth;
    svg.setAttribute("width", overallWidth);
    svg.setAttribute("height", (treeHeight(tree) - 0.3) * levelHeight);

    var nodeList = [];
    buildPQNode(nodeList, tree, overallWidth / 2, 20);
    tikzOut = document.getElementById("tikzOut");
    tikzOut.innerHTML = "\\begin{tikzpicture}[leaf/.style={fill=black!10},pnode/.style={leaf},qnode/.style={leaf}]\n";
    for (var node of nodeList) {
        svg.appendChild(nodeToSvg(node));
        tikzOut.innerHTML += "  " + nodeToTikZ(node) + "\n";
    }
    tikzOut.innerHTML += "\\end{tikzpicture}";

    var computedHeight = Math.max(...yValuesWhileRendering) + 5;
    svg.setAttribute("height", computedHeight);
    svg.setAttribute('viewBox', `0 0 ${overallWidth} ${computedHeight}`);

    var svgData = svg.outerHTML;
    var svgBlob = new Blob([svgData], { type: "image/svg+xml;charset=utf-8" });
    var svgUrl = URL.createObjectURL(svgBlob);
    downloadLink.innerHTML = "Download SVG";
    downloadLink.href = svgUrl;
    downloadLink.download = "newesttree.svg";
}

const factorial = [1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800, 39916800];
function numberOfEncodedOrderings(node) {
    if (node == null) {
        return 0;
    } else if (node.type == "leaf") {
        return 1;
    } else if (node.type == "pnode") {
        var count = 1;
        for (var child of node.children) {
            count *= numberOfEncodedOrderings(child);
        }
        return count * factorial[node.children.length];
    } else if (node.type == "qnode") {
        var count = 1;
        for (var child of node.children) {
            count *= numberOfEncodedOrderings(child);
        }
        return count * 2;
    } else {
        throw "Unknown node type " + node.type;
    }
}

function allOrderings(node) {
    function permutations(permutation) {
        var length = permutation.length,
            result = [permutation.slice()],
            c = new Array(length).fill(0),
            i = 1, k, p;

        while (i < length) {
            if (c[i] < i) {
                k = i % 2 && c[i];
                p = permutation[i];
                permutation[i] = permutation[k];
                permutation[k] = p;
                ++c[i];
                i = 1;
                result.push(permutation.slice());
            } else {
                c[i] = 0;
                ++i;
            }
        }
        return result;
    }

    const flatten = (arr) => [].concat.apply([], arr);
    const product = (...sets) =>
        sets.reduce((acc, set) =>
                flatten(acc.map(x => set.map(y => [...x, y]))),
            [[]]);


    if (node == null) {
        return [];
    } else if (node.type == "leaf") {
        return [[node.value]];
    } else if (node.type == "pnode") {
        var childOrderings = [];
        for (var child of node.children) {
            childOrderings.push(allOrderings(child));
        }
        var orderings = [];
        for (var perm of permutations(range(node.children.length))) {
            for (var prod of product(...childOrderings)) {
                var ordering = range(node.children.length).map(i => prod[perm[i]]);
                orderings.push(ordering.flat());
            }
        }
        return orderings;
    } else if (node.type == "qnode") {
        var childOrderings = [];
        for (var child of node.children) {
            childOrderings.push(allOrderings(child));
        }
        var orderings = [];
        for (var prod of product(...childOrderings)) {
            orderings.push(prod);
            // copy prod
            var prod2 = prod.map(x => x);
            prod2.reverse();
            orderings.push(prod2);
        }
        return orderings;
    } else {
        throw "Unknown node type " + node.type;
    }
}
