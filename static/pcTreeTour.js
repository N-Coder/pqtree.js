function setMatrix(matrix, circular = false) {
    return function (currentStep, nextStep) {
        document.getElementById("toggle-cyclic").checked = circular;
        data = matrix;
        buildTable();
        update();
    }
}

const tg = new tourguide.TourGuideClient({
    debug: true,
    steps: [{
        target: "#start-tour",
        title: "Hey there! 👋",
        content: `This is a short guide to teach you what PQ- and PC-trees are and how to use this site.\n
            If you already know your way around, feel free to close this.
            You can always reopen the guide by clicking this button.`,
    }, {
        target: "#input-table-wrapper",
        title: "Use-Case",
        content: `We want to reorder the columns of this <a href="https://doi.org/10.1006/jagm.2001.1205">binary matrix</a> such that,
            in each row, the selected cells are consecutive (together and not interrupted by white cells).
            To do this, we can use a datastructure called <a href="https://en.wikipedia.org/wiki/PQ_tree">PQ-tree</a>.`,
    }, {
        target: "#svg-container",
        title: "The PQ-tree",
        content: `The leaves of this PQ-tree correspond to the columns of the matrix,
            their order indicates one possible (re)ordering with the desired consecutivities.
            There are two types of inner nodes:
            Round <i>P-nodes</i> allow arbitrarily changing the order of their children.
            Rectangular <i>Q-nodes</i> only allow reversing the order of their children.
            Hover over any node to see its corresponding column(s) in the matrix.`,
    }, {
        target: "#main-content",
        title: "A trivial example",
        content: `If no row of the matrix contains two or more cells that need to be together, any order of columns will do.
            This is represented by a <i>trivial</i> PQ-tree, where a single P-node allows arbitrarily permuting the leaves.`,
        beforeEnter: setMatrix([
            [0, 1, 0, 0, 0],
            [0, 0, 0, 0, 0],
            [1, 0, 0, 0, 0],
            [0, 0, 1, 0, 0],
            [0, 0, 0, 0, 1],
        ])
    }, {
        target: "#main-content",
        title: "A first simple constraint",
        content: `If we add a single consecutivity constraint, the P-node gets split in half.
            Observe that the new P-node ensures that leaves from the consecutive set cannot interleave (mix) with the others.`,
        beforeEnter: setMatrix([
            [0, 1, 0, 0, 0],
            [0, 0, 0, 0, 0],
            [1, 0, 0, 0, 0],
            [0, 0, 1, 1, 0],
            [0, 0, 0, 0, 1],
        ])
    }, {
        target: "#main-content",
        title: "Another one",
        content: `Adding a new row that uses different columns also adds a further new P-node.`,
        beforeEnter: setMatrix([
            [0, 1, 0, 0, 0],
            [0, 0, 0, 0, 0],
            [1, 0, 0, 0, 0],
            [0, 0, 1, 1, 0],
            [0, 0, 0, 0, 1],
            [1, 1, 0, 0, 0],
        ])
    }, {
        target: "#main-content",
        title: "The third constraint",
        content: `If we now add a third constraint that makes some leaves from the two previous constraints consecutive, there is a bigger change to the PQ-tree.
            The new Q-node ensures that we have first all leaves from the first constraint, and then those from the second constraint, uninterrrupted by any leaves that are part of neither constraint.
            It further ensures that the leaves of our new constraint are in the middle between both sets and in the right order.`,
        beforeEnter: setMatrix([
            [0, 1, 0, 0, 0],
            [0, 0, 0, 0, 0],
            [1, 0, 0, 0, 0],
            [0, 0, 1, 1, 0],
            [0, 0, 0, 0, 1],
            [1, 1, 0, 0, 0],
            [0, 1, 1, 0, 0],
        ])
    }, {
        target: "#main-content",
        title: "An impossible constraint",
        content: `If we now wanted to also make these three leaves consecutive, we would no longer be able to find an order that satisfies all consecutivity constraints.
            This indicated by the update adding this last constraint to the PQ-tree failing.`,
        beforeEnter: setMatrix([
            [0, 1, 0, 0, 0],
            [0, 0, 0, 0, 0],
            [1, 0, 0, 0, 0],
            [0, 0, 1, 1, 0],
            [0, 0, 0, 0, 1],
            [1, 1, 0, 0, 0],
            [0, 1, 1, 0, 0],
            [1, 0, 0, 1, 1],
        ])
    }, {
        target: "#main-content",
        title: "The update algorithm",
        content: `The original algorithm to determine updates on PQ-trees is defined in terms of various templates that need to be recursively matched:
            [link]
            Fortunately, there is a slightly different perspective that makes updates far easier.`,
    }, {
        target: "#main-content",
        title: "Circular Orders and PC-trees",
        content: `What if we allowed selected cells to either appear consecutively in the middle, or split into two parts at the very beggining and end (and thus have the not selected cells consecutive in the middle)?
            This now allows us to find a order that also satisfies the last constraint.
            While the previous (linear) orders hat a unique start and end, this new order is cyclic and can be shifted arbitrarily.`,
    }, {
        target: "#main-content",
        title: "another one",
        content: ``,
    }].toReversed()
})
