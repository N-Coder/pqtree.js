function setMatrix(new_matrix, new_titles = [], circular = false) {
    return function (currentStep, nextStep) {
        if (block_url_updates) return;
        document.getElementById("toggle-cyclic").checked = circular;
        titles = new_titles;
        data = new_matrix;
        let old_buu = block_url_updates;
        block_url_updates = true;
        buildTable();
        update();
        block_url_updates = old_buu;
    }
}

const tg = new tourguide.TourGuideClient({
    propagateEvents: true,
    debug: false,
    completeOnFinish: false,
    dialogMaxWidth: 400,
    steps: [{
        target: "#start-tour",
        title: "Hey there! 👋",
        content: `This is a short guide to teach you what PQ- and PC-trees are and how to use this site.
            If you already know your way around, feel free to close this.<br/><br/>
            Note that you can always undo changes to the matrix or go back in this tutorial by pressing your
            browser's back button.`,
        afterEnter: writeURL
    }, {
        title: "The Problem",
        content: `Say you are planning a trip to the cinema for a school class.
            You booked a full row for all students, you just need to <a target="_blank" href="https://xkcd.com/173/">figure out who sits where</a>.<br/>
            <img src="assets/rows.svg"/><br/>
            There are of course some restrictions on who wants to sit with whom.
            There is for example one group of friends that definitely want to sit together.<br/>
            <img src="assets/constraint1.svg"/><br/>
            Also, there are two couples who will, respectively, want to sit next to each other.<br/>
            <img src="assets/constraint2.svg"/><br/>
            <img src="assets/constraint3.svg"/><br/>
            And there is a pair of best friends between the two couples which we need to place next to each other.<br/>
            <img src="assets/constraint4.svg"/><br/>
            While the images readily show one possible seating order, you wonder if there is a structured way to enumerate all admissible orders.`
    }, {
        target: "#input-table-wrapper",
        title: "A Model",
        content: `We will model this problem through a <i>binary matrix</i>, where the <i>columns</i> correspond to the students for which we want to find a seating order.
            We add one <i>row</i> for each group of people that want to be seated together, setting exactly their entries in the row to 1.
            The problem is now to find a reordering of the columns of the matrix such that, in every row, all ones are <i>consecutive</i>.
            This problem is also known as <a target="_blank" href="https://doi.org/10.1016/S0022-0000(76)80045-1">Consecutive 1's Problem (C1P)</a>.<br/><br/>
            
            You can try finding such orders yourself by dragging the column headings left and right, but we'll look at more systematic way to do this in a second.
            You can also toggle entries by clicking on them.
            Recall that you can also undo changes to the matrix and/or go back to the previous tutorial step using your browser's back button.`,
        beforeEnter: setMatrix([
            // [0,0,0,0,0,0,0,0,0],
            [0, 1, 1, 0, 0, 1, 1, 1, 0],
            [1, 0, 0, 0, 1, 0, 0, 0, 0],
            [0, 0, 0, 0, 0, 0, 1, 1, 0],
            [1, 0, 0, 0, 0, 0, 0, 1, 0],
            [0, 0, 0, 1, 1, 0, 0, 0, 1],
            // [0,1,0,0,0,0,0,0,1],
        ])
    }, {
        target: "#svg-container",
        title: "The Solution",
        content: `To solve the Consecutive 1's Problem in a structured way, we can use a data structure called <a target="_blank" href="https://en.wikipedia.org/wiki/PQ_tree">PQ-tree</a>.
            Its leaves correspond to the elements we want to order.
            They are connected through two different types of inner nodes that describe how the leaves can be reordered:
            The first type are the round <i>P-nodes</i>, which allow arbitrarily changing the order of their children.
            So the rooted tree as it currently is would allow all permutations of students, with no regard to their seating preferences.`,
        beforeEnter: setMatrix([
            [0, 0, 0, 0, 0, 0, 0, 0, 0],
        ])
    }, {
        target: "#main-content",
        title: "Adding Restrictions",
        content: `If we now want to ensure that the friend group <tt>b,c,f,g,h</tt> is consecutive, we add a row to the matrix where exactly those entries are set to 1.
            The PQ-tree is automatically updated such that its represented orders are restricted to those ones where <tt>b,c,f,g,h</tt> are consecutive.
            In this case, this splits the central P-node into two P-nodes, that now prevent that their respective leaves getting mixed.<br/><br/>
            
            Hover over the nodes to see which columns they correspond to.`,
        beforeEnter: setMatrix([
            [0, 0, 0, 0, 0, 0, 0, 0, 0],
            [0, 1, 1, 0, 0, 1, 1, 1, 0],
            // [1,0,0,0,1,0,0,0,0],
            // [0,0,0,0,0,0,1,1,0],
            // [1,0,0,0,0,0,0,1,0],
            // [0,0,0,1,1,0,0,0,1],
            // [0,1,0,0,0,0,0,0,1],
        ])
    }, {
        target: "#main-content",
        title: "The Next Restriction",
        content: `Adding the first couple <tt>a,e</tt> further splits off another P-node.`,
        beforeEnter: setMatrix([
            [0, 0, 0, 0, 0, 0, 0, 0, 0],
            [0, 1, 1, 0, 0, 1, 1, 1, 0],
            [1, 0, 0, 0, 1, 0, 0, 0, 0],
            // [0,0,0,0,0,0,1,1,0],
            // [1,0,0,0,0,0,0,1,0],
            // [0,0,0,1,1,0,0,0,1],
            // [0,1,0,0,0,0,0,0,1],
        ])
    }, {
        target: "#main-content",
        title: "Another Restriction",
        content: `The same happens for the second couple <tt>g,h</tt>, which are both part of <tt>b,c,f,g,h</tt> friend group.<br/><br/>

            Up to now all restriction simply split P-nodes because their elements belonged to the same parent P-node.
            If we now add the best friends pair <tt>a,h</tt> in the next step, we'll need to make bigger changes to the tree...`,
        beforeEnter: setMatrix([
            [0, 0, 0, 0, 0, 0, 0, 0, 0],
            [0, 1, 1, 0, 0, 1, 1, 1, 0],
            [1, 0, 0, 0, 1, 0, 0, 0, 0],
            [0, 0, 0, 0, 0, 0, 1, 1, 0],
            // [1,0,0,0,0,0,0,1,0],
            // [0,0,0,1,1,0,0,0,1],
            // [0,1,0,0,0,0,0,0,1],
        ])
    }, {
        target: "#main-content",
        title: "A Restriction Between Nodes",
        content: `To add the best friends pair <tt>a,h</tt>, we now need the second type of inner node: 
            While round <i>P-nodes</i> allow arbitrarily changing the order of their children, rectangular <i>Q-nodes</i> only allow reversing the order of their children.<br/><br/>
            
            The new Q-node now ensures that we have first the <tt>e,a</tt> couple, joined by the best friends <tt>a,h</tt> with the <tt>h,g</tt> couple, and then the rest of the <tt>g,h,b,c,f</tt> group in arbitrary order.
            Due to the couples and best friends, we can only reverse this order.
            Note that in any order, the <tt>b,c,f,g,h</tt> group still sits together.`,
        beforeEnter: setMatrix([
            [0, 0, 0, 0, 0, 0, 0, 0, 0],
            [0, 1, 1, 0, 0, 1, 1, 1, 0],
            [1, 0, 0, 0, 1, 0, 0, 0, 0],
            [0, 0, 0, 0, 0, 0, 1, 1, 0],
            [1, 0, 0, 0, 0, 0, 0, 1, 0],
            // [0,0,0,1,1,0,0,0,1],
            // [0,1,0,0,0,0,0,0,1],
        ])
    }, {
        target: "#aside-main",
        title: "Finding Admissible Orders",
        content: `To now use the PQ-tree to find a column order that has all 1's consecutive, you can press the "Reorder matrix to consecutive" button.
            You can also see how many different admissible orders the PQ-tree/matrix have, which we can enumerate by going through all permutations of children for each P-node and by flipping each C-node.`,
        beforeEnter: () => {
            document.getElementById("hint-orderings").open = true
        }
    }, {
        target: "#main-content",
        title: "Adding More Restrictions",
        content: `Say student <tt>i</tt> just told you they want to sit together with <tt>d,e</tt>.
            As the reordered matrix shows, we can still easily accommodate this restriction, 
            although with the single large Q-node we can also see that most of the seating order is by now already fixed.`,
        beforeEnter: setMatrix([
            [0, 0, 0, 0, 0, 0, 0, 0, 0],
            [0, 0, 0, 0, 1, 1, 1, 1, 1],
            [0, 0, 1, 1, 0, 0, 0, 0, 0],
            [0, 0, 0, 0, 1, 1, 0, 0, 0],
            [0, 0, 0, 1, 1, 0, 0, 0, 0],
            [1, 1, 1, 0, 0, 0, 0, 0, 0],
        ], ['d', 'i', 'e', 'a', 'h', 'g', 'b', 'c', 'f'])
    }, {
        target: "#main-content",
        title: "An Impossible Restriction",
        content: `If you now also heard that <tt>f,d</tt> want to sit next to each other, you'd run into a problem.
            With this additional restriction, there is no order that satisfies all constraints.<br/><br/>

            The tree shown represents all restrictions up to the unsatisfiable one shown in red.
            You can drag the rows by their numbers to rearrange them, although you won't find an order for the rows such that all can be satisfied.
            If all constraints could be satisfied, the order in which they are added would be irrelevant.`,
        beforeEnter: setMatrix([
            [0, 0, 0, 0, 0, 0, 0, 0, 0],
            [0, 0, 0, 0, 1, 1, 1, 1, 1],
            [0, 0, 1, 1, 0, 0, 0, 0, 0],
            [0, 0, 0, 0, 1, 1, 0, 0, 0],
            [0, 0, 0, 1, 1, 0, 0, 0, 0],
            [1, 1, 1, 0, 0, 0, 0, 0, 0],
            [1, 0, 0, 0, 0, 0, 0, 0, 1],
        ], ['d', 'i', 'e', 'a', 'h', 'g', 'b', 'c', 'f'], false)
    }, {
        target: "#hint-cyclic",
        title: "Cyclic Orders",
        content: `There might be a solution: the 3-D cinema dome doesn't have chairs in rows, but in a circle.
            Going from our previously linear seating order to such cyclic one would allow the first and last persons in the row to now sit next to each other in the cycle.
            In our matrix model, this would allow us to still consider a row satisfied if all its ones wrap around, that is, they are split between its very beginning and end.
            This variant of our problem is called the <a target="_blank" href="https://doi.org/10.1016/S0304-3975(02)00435-8">Circular Consecutive 1's Problem</a> (CC1P) and you can use this checkbox to switch to it.<br/>
            <img style="padding-left: 70px" src="assets/circle.svg"/>`,
        beforeEnter: () => {
            document.getElementById("hint-cyclic").open = true;
            document.getElementById("toggle-cyclic").checked = false;
        }
    }, {
        target: "#main-content",
        title: "The PC-tree",
        content: `Instead of the linear PQ-tree, we will now also use its cyclic counterpart, the <a target="_blank" href="https://en.wikipedia.org/wiki/PQ_tree#PC_trees">PC-tree</a>.
            It still has leaves and P-nodes, the only difference is that Q-nodes got renamed to C-nodes and we no longer have a fixed starting point when reading off the order of leaves.
            Other than that, PQ- and PC-trees work the same, they are actually <a target="_blank" href="https://doi.org/10.1016/j.endm.2008.06.029">linear-time equivalent</a>.`,
        beforeEnter: setMatrix([
            [0, 0, 0, 0, 0, 0, 0, 0, 0],
            [0, 0, 0, 0, 1, 1, 1, 1, 1],
            [0, 0, 1, 1, 0, 0, 0, 0, 0],
            [0, 0, 0, 0, 1, 1, 0, 0, 0],
            [0, 0, 0, 1, 1, 0, 0, 0, 0],
            [1, 1, 1, 0, 0, 0, 0, 0, 0],
            [1, 0, 0, 0, 0, 0, 0, 0, 1],
        ], ['d', 'i', 'e', 'a', 'h', 'g', 'b', 'c', 'f'], true)
    }, {
        title: "The End",
        content: `This concludes the tour of this site.
        If you want to learn more about PQ- & PC-trees, especially how a tree updated with new constraints is computed,
        you can also check out this <a target="_blank" href="https://tryalgo.org/en/data%20structures/2024/01/03/pc-trees/">tryalgo article</a>
        (or its <a target="_blank" href="https://tryalgo.org/en/data%20structures/2017/12/15/pq-trees/">prior version</a> focusing on PQ-trees).
        PQ-trees were initially introduced by 
            <a target="_blank" href="https://doi.org/10.1016/S0022-0000(76)80045-1">Booth and Lueker</a>
        and PC-trees by 
            <a target="_blank" href="https://doi.org/10.1016/S0304-3975(02)00435-8">Hsu and McConnell</a>.
        A more modern explanation of both data structures, their workings and their differences can be found starting on page 10 of this
            <a target="_blank" href="https://doi.org/10.15475/cpatp.2024">thesis</a>.`,
    }].toReversed()
})

tg.onAfterExit(writeURL)
tg.onAfterStepChange(writeURL)
