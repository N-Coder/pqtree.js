#pragma once

#include <regex>
#include <tuple>
#include <variant>

#include "layout.h"

using namespace pc_tree;

struct Drawer {
	double offset_x;
	double offset_y;

	PCTreeNodeArray<std::string>* labels;

	virtual ~Drawer() = default;
	virtual void draw(const PCTree& tree, const Layout& layout, std::stringstream& ss) = 0;
	virtual void drawLine(std::stringstream& ss, double x1, double y1, double x2, double y2) = 0;
	virtual void drawLeaf(std::stringstream& ss, double x, double y, std::string label) = 0;
	virtual void drawPNode(std::stringstream& ss, double x, double y) = 0;

	virtual void writeHeader(std::stringstream& ss) { }

	virtual void writeTrailer(std::stringstream& ss) { }

	virtual std::string getLabel(PCNode* node) {
		if (labels) {
			return (*labels)[node];
		} else {
			return std::to_string(node->index());
		}
	}

	virtual std::string getLeaves(PCNode* below) {
		std::stringstream ss;
		bool first = true;
		for (PCNode* leaf : FilteringPCTreeDFS(*cur_tree, below)) {
			if (leaf->isLeaf()) {
				if (first) {
					first = false;
				} else {
					ss << " ";
				}
				ss << getLabel(leaf);
			}
		}
		return ss.str();
	}

	virtual std::stringstream& writeXMLNode(std::stringstream& ss, const std::string& tag,
			std::initializer_list<std::tuple<std::string, std::variant<std::string, double, int>>> values);

protected:
	const PCTree* cur_tree;
	PCNode* cur_node;
};

struct LinearDrawer : virtual Drawer {
	PCTreeNodeArray<double>* widths;
	double width;
	double height;

	int pNode_yShift = 15;

	virtual void drawQNode(std::stringstream& ss, double x, double y, double width, double height) = 0;

	void draw(const PCTree& tree, const Layout& positions, std::stringstream& ss) override;

	virtual std::tuple<std::string, double> getTrianglePath(double cx, double cy);

	virtual std::tuple<double, double, double, double> getQNodeSize(PCNode* node,
			const Layout& positions);
};

struct CircularDrawer : virtual Drawer {
	double radius;

	void draw(const PCTree& tree, const Layout& positions, std::stringstream& ss) override;

	virtual void drawCNode(std::stringstream& ss, double x, double y) = 0;
};

// /////////////////////////////////////////////////////////////////////////////////////////////////
// SVG /////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////

struct SVGDrawer : virtual Drawer {
	void writeTrailer(std::stringstream& ss) override { ss << "</svg>"; }

	void drawLine(std::stringstream& ss, double x1, double y1, double x2, double y2) override {
		writeXMLNode(ss, "line",
				{{"x1", x1}, {"y1", y1}, {"x2", x2}, {"y2", y2}, {"stroke", "black"}});
	}
};

struct SVGCircularDrawer : virtual SVGDrawer, virtual CircularDrawer {
	double leaf_radius = 15;

	void writeHeader(std::stringstream& ss) override {
		double act_rad = offset_x = offset_y = radius + leaf_radius + 1;
		double size = act_rad * 2;
		ss << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << size << "\" height=\"" << size
		   << "\">";
		writeXMLNode(ss, "circle",
				{{"cx", offset_x}, {"cy", offset_y}, {"r", radius}, {"fill", "transparent"},
						{"stroke", "#cccccc"}, {"pointer-events", "none"}});
	}

	void drawLeaf(std::stringstream& ss, double cx, double cy, std::string label) override {
		writeXMLNode(ss, "circle",
				{{"cx", cx}, {"cy", cy}, {"r", leaf_radius}, {"fill", "white"},
						{"stroke", "#666666"}, {"data-leaves", label}});
		writeXMLNode(ss, "text",
				{{"x", cx}, {"y", cy}, {"text-anchor", "middle"}, {"dominant-baseline", "central"},
						{"text", label}});
	}

	void drawPNode(std::stringstream& ss, double cx, double cy) override {
		writeXMLNode(ss, "circle",
				{{"cx", cx}, {"cy", cy}, {"r", "5"}, {"fill", "black"}, {"stroke", "black"}});
	}

	void drawCNode(std::stringstream& ss, double cx, double cy) override {
		writeXMLNode(ss, "circle",
				{{"cx", cx}, {"cy", cy}, {"r", "20"}, {"fill", "#ececec"}, {"stroke", "black"}});
		writeXMLNode(ss, "circle",
				{{"cx", cx}, {"cy", cy}, {"r", "15"}, {"fill", "transparent"}, {"stroke", "black"},
						{"pointer-events", "none"}});
		writeXMLNode(ss, "text",
				{{"x", (cx - 1)}, {"y", cy}, {"text-anchor", "middle"},
						{"dominant-baseline", "central"}, {"text", "C"}});
	}
};

struct SVGLinearDrawer : virtual SVGDrawer, virtual LinearDrawer {
	void writeHeader(std::stringstream& ss) override {
		ss << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << width << "\" height=\""
		   << height << "\">";
	}

	void drawLeaf(std::stringstream& ss, double cx, double cy, std::string label) override {
		auto [tri, mid_y] = getTrianglePath(cx, cy);
		writeXMLNode(ss, "polygon",
				{{"points", tri}, {"fill", "white"}, {"stroke", "black"}, {"data-leaves", label}});
		writeXMLNode(ss, "text",
				{{"x", cx}, {"y", mid_y}, {"text-anchor", "middle"},
						{"dominant-baseline", "central"}, {"text", label}});
	}

	void drawPNode(std::stringstream& ss, double cx, double cy) override {
		writeXMLNode(ss, "circle",
				{{"cx", cx}, {"cy", cy}, {"r", "15"}, {"fill", "#ececec"}, {"stroke", "black"},
						{"data-leaves", getLeaves(cur_node)}});
		writeXMLNode(ss, "text",
				{{"x", (cx + 0.4)}, {"y", cy}, {"text-anchor", "middle"},
						{"dominant-baseline", "central"}, {"text", "P"}});
	}

	void drawQNode(std::stringstream& ss, double x, double y, double width, double height) override {
		writeXMLNode(ss, "rect",
				{{"x", x}, {"y", y}, {"width", width}, {"height", height}, {"fill", "#ececec"},
						{"stroke", "black"}, {"data-leaves", getLeaves(cur_node)}});
		writeXMLNode(ss, "text",
				{{"x", x + width / 2}, {"y", y + height / 2}, {"text-anchor", "middle"},
						{"dominant-baseline", "central"}, {"text", "Q"}});
	}
};

// /////////////////////////////////////////////////////////////////////////////////////////////////
// IPE /////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////

struct IPEDrawer : virtual Drawer {
	void writeHeader(std::stringstream& ss) override {
		ss << "<?xml version=\"1.0\"?>\n<!DOCTYPE ipe SYSTEM \"ipe.dtd\">\n<ipe version=\"70218\" creator=\"pqtree.js\">\n";
		ss << "<page>\n<layer name=\"alpha\"/>\n<view layers=\"alpha\" active=\"alpha\"/>"
		   << std::endl;
	}

	void writeTrailer(std::stringstream& ss) override { ss << "</page>\n</ipe>" << std::endl; }

	void drawLine(std::stringstream& ss, double x1, double y1, double x2, double y2) override {
		ss << "<path>\n"
		   << x1 << " " << y1 << " m\n"
		   << x2 << " " << y2 << " l\n</path>" << std::endl;
	}

	virtual void drawMark(std::stringstream& ss, double x, double y, std::string mark) {
		ss << "<use matrix=\"0 0 0 0 " << x << " " << y << "\" name=\"mark/" << mark << "\"/>"
		   << std::endl;
	}

	virtual void drawText(std::stringstream& ss, double x, double y, std::string text) {
		writeXMLNode(ss, "text",
				{{"text", text}, {"pos", std::to_string(x) + " " + std::to_string(y)},
						{"type", "label"}, {"halign", "center"}, {"valign", "center"}})
				<< std::endl;
	}

	virtual std::string getCirclePath(double rad, double x, double y) {
		std::stringstream circ;
		circ << rad << " 0 0 " << rad << " " << x << " " << y << " e";
		return circ.str();
	}
};

struct IPECircularDrawer : virtual IPEDrawer, virtual CircularDrawer {
	void writeHeader(std::stringstream& ss) override {
		IPEDrawer::writeHeader(ss);
		double act_rad = offset_x = offset_y = radius + 15 + 1;
		writeXMLNode(ss, "path",
				{{"stroke", "black"}, {"text", getCirclePath(act_rad, offset_x, offset_y)}})
				<< std::endl;
	}

	void drawLeaf(std::stringstream& ss, double x, double y, std::string label) override {
		drawMark(ss, x, y, "leaf(sfx)");
		drawText(ss, x, y, label);
	}

	void drawPNode(std::stringstream& ss, double x, double y) override {
		drawMark(ss, x, y, "pnode(sfx)");
	}

	void drawCNode(std::stringstream& ss, double x, double y) override {
		drawMark(ss, x, y, "cnode(sfx)");
	}
};

struct IPELinearDrawer : virtual IPEDrawer, virtual LinearDrawer {
	void drawLeaf(std::stringstream& ss, double cx, double cy, std::string label) override {
		auto [tri, mid_y] = getTrianglePath(cx, cy);
		writeXMLNode(ss, "path", {{"text", tri}}) << std::endl;
		drawText(ss, cx, mid_y, label);
	}

	void drawPNode(std::stringstream& ss, double cx, double cy) override {
		writeXMLNode(ss, "path", {{"stroke", "black"}, {"text", getCirclePath(15, cx, cy)}})
				<< std::endl;
		drawText(ss, cx, cy, "P");
	}

	void drawQNode(std::stringstream& ss, double x, double y, double width, double height) override {
		ss << "<path>\n"
		   << x << " " << y << " m\n"
		   << x << " " << (y + height) << " l\n"
		   << (x + width) << " " << (y + height) << " l\n"
		   << (x + width) << " " << y << " l\nh\n</path>" << std::endl;
	}
};

// /////////////////////////////////////////////////////////////////////////////////////////////////
// Tikz ////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////

struct TikzDrawer : virtual Drawer {
	void writeTrailer(std::stringstream& ss) override { ss << "\\end{tikzpicture}"; }

	void drawLine(std::stringstream& ss, double x1, double y1, double x2, double y2) override {
		ss << "\\draw (" << (x1) << "," << (-y1) << ") -- (" << (x2) << "," << (-y2) << ");"
		   << std::endl;
	}
};

struct TikzCircularDrawer : virtual TikzDrawer, virtual CircularDrawer {
	double leaf_radius = 15;

	void writeHeader(std::stringstream& ss) override {
		ss << "\\begin{tikzpicture}[leaf/.style={fill=black!10},pnode/"
			  ".style={leaf},qnode/.style={leaf}]\n";
		double act_rad = offset_x = offset_y = radius + leaf_radius + 1;
		ss << "\\draw[background] (" << offset_x << "," << -offset_y
		   << ") circle [radius=" << act_rad << "];" << std::endl;
	}

	void drawLeaf(std::stringstream& ss, double cx, double cy, std::string label) override {
		ss << "\\draw[leaf] (" << cx << "," << -cy << ") circle [radius=" << leaf_radius << "];"
		   << std::endl;
		ss << "\\node at (" << cx << "," << -cy << ") {" << label << "};" << std::endl;
	}

	void drawPNode(std::stringstream& ss, double cx, double cy) override {
		ss << "\\draw[pnode] (" << cx << "," << -cy << ") circle [radius=" << 5 << "];" << std::endl;
	}

	void drawCNode(std::stringstream& ss, double cx, double cy) override {
		ss << "\\draw[cnode] (" << cx << "," << -cy << ") circle [radius=" << 19 << "];" << std::endl;
	}
};

struct TikzLinearDrawer : virtual TikzDrawer, virtual LinearDrawer {
	void writeHeader(std::stringstream& ss) override {
		ss << "\\begin{tikzpicture}[leaf/.style={fill=black!10},pnode/"
			  ".style={leaf},qnode/.style={leaf}]"
		   << std::endl;
	}

	void drawLeaf(std::stringstream& ss, double cx, double cy, std::string label) override {
		auto [tri, mid_y] = getTrianglePath(cx, cy);
		tri = std::regex_replace(tri, std::regex(" "), ") -- (");
		tri = std::regex_replace(tri, std::regex(","), ",-");
		ss << "\\draw[leaf](" << tri << ") -- cycle;" << std::endl;
		ss << "\\node at (" << (cx) << "," << -mid_y << ") {" << label << "};" << std::endl;
	}

	void drawPNode(std::stringstream& ss, double cx, double cy) override {
		ss << "\\draw[pnode] (" << cx << "," << -cy << ") circle [radius=" << 15 << "];" << std::endl;
	}

	void drawQNode(std::stringstream& ss, double x, double y, double width, double height) override {
		ss << "\\draw[qnode] (" << x << "," << -y << ") rectangle (" << x + width << ","
		   << -y - height << ");" << std::endl;
	}
};
