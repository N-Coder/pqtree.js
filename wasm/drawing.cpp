#include "drawing.h"

void LinearDrawer::draw(const PCTree& tree, const Layout& positions, std::stringstream& ss) {
	cur_tree = &tree;
	writeHeader(ss);
	for (auto node : tree.allNodes()) {
		cur_node = node;
		if (node->isLeaf() && node == tree.getRootNode()) {
			continue;
		}

		auto [cx, cy] = positions[node];
		cx += offset_x;
		cy += offset_y;
		if (node->isLeaf()) {
			drawLeaf(ss, cx, cy, getLabel(node));
			continue;
		}

		if (node->getNodeType() == PCNodeType::PNode && node != tree.getRootNode()) {
			cy += pNode_yShift;
		}

		for (auto child : node->children()) {
			auto [childCX, childCY] = positions[child];
			childCX += offset_x;
			childCY += offset_y;
			if (child->getNodeType() == PCNodeType::PNode) {
				childCY += pNode_yShift;
			}
			drawLine(ss, (node->getNodeType() == PCNodeType::PNode ? cx : childCX), cy, childCX,
					childCY);
		}

		if (node->getNodeType() == PCNodeType::PNode) {
			drawPNode(ss, cx, cy);
		} else {
			auto [x, y, width, height] = getQNodeSize(node, positions);
			drawQNode(ss, x, y, width, height);
		}
	}
	cur_node = nullptr;
	writeTrailer(ss);
	cur_tree = nullptr;
}

std::tuple<std::string, double> LinearDrawer::getTrianglePath(double cx, double cy) {
	// cy -= 15;
	double ratio = 0.866; // equilateral triangle
	double sideLength = 40;
	std::stringstream points;
	points << cx << "," << cy << " ";
	points << cx - sideLength / 2 << "," << cy + sideLength * ratio << " ";
	points << cx + sideLength / 2 << "," << cy + sideLength * ratio;
	double mid_y = cy + 0.69 * sideLength * ratio;
	return {points.str(), mid_y};
}

std::tuple<double, double, double, double> LinearDrawer::getQNodeSize(PCNode* node,
		const Layout& positions) {
	auto [cx, cy] = positions[node];
	cx += offset_x;
	cy += offset_y;

	auto height = 0.3 * 80; // levelHeight
	double left_x = 0, width = 0;
	if (widths && (*widths)[node] > 0) {
		auto buffer = 0.1 * 70; // leafWidth
		width = (*widths)[node];
		left_x = cx - width / 2 + buffer;
		width -= 2 * buffer;
	} else {
		auto buffer = 0.4 * 70; // leafWidth
		double first_child_x = std::get<0>(positions[node->getChild1()]) + offset_x;
		double last_child_x = std::get<0>(positions[node->getChild2()]) + offset_x;
		if (first_child_x > last_child_x) {
			std::swap(first_child_x, last_child_x);
		}
		left_x = first_child_x - buffer;
		width = (last_child_x - first_child_x) + 2 * buffer;
	}
	return {left_x, cy, width, height};
}

void CircularDrawer::draw(const PCTree& tree, const Layout& positions, std::stringstream& ss) {
	cur_tree = &tree;
	writeHeader(ss);
	for (auto node : tree.allNodes()) {
		cur_node = node;
		auto [cx, cy] = positions[node];
		cx += offset_x;
		cy += offset_y;
		if (node->isLeaf()) {
			drawLeaf(ss, cx, cy, getLabel(node));
			continue;
		}

		for (auto child : node->children()) {
			auto [childCX, childCY] = positions[child];
			childCX += offset_x;
			childCY += offset_y;
			drawLine(ss, cx, cy, childCX, childCY);
		}

		if (node->getNodeType() == PCNodeType::PNode) {
			drawPNode(ss, cx, cy);
		} else {
			drawCNode(ss, cx, cy);
		}
	}
	cur_node = nullptr;
	writeTrailer(ss);
	cur_tree = nullptr;
}

std::stringstream& Drawer::writeXMLNode(std::stringstream& ss, const std::string& tag,
		std::initializer_list<std::tuple<std::string, std::variant<std::string, double, int>>> values) {
	std::string text;
	ss << "<" << tag << " ";
	for (auto [key, var] : values) {
		if (key == "text") {
			text = std::get<std::string>(var);
			continue;
		}
		std::string value;
		if (std::holds_alternative<int>(var)) {
			value = std::to_string(std::get<int>(var));
		} else if (std::holds_alternative<double>(var)) {
			value = std::to_string(std::get<double>(var));
		} else {
			value = std::get<std::string>(var);
		}
		ss << key << "=\"" << value << "\" ";
	}
	if (!text.empty()) {
		ss << ">" << text << "</" << tag << ">";
	} else {
		ss << "/>";
	}
	return ss;
}
