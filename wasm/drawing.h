#pragma once

#include <regex>
#include <tuple>
#include <variant>

#include "layout.h"

using namespace pc_tree;

struct Drawer {
	double offset_x;
	double offset_y;
	double node_size;

	PCTreeNodeArray<std::string>* labels;

	virtual ~Drawer() = default;
	virtual void draw(const PCTree& tree, const Layout& layout, std::ostream& ss) = 0;
	virtual void drawLine(std::ostream& ss, double x1, double y1, double x2, double y2) = 0;
	virtual void drawLeaf(std::ostream& ss, double x, double y, std::string label) = 0;
	virtual void drawPNode(std::ostream& ss, double x, double y) = 0;

	virtual void writeHeader(std::ostream& ss) { }

	virtual void writeTrailer(std::ostream& ss) { }

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

	virtual std::ostream& writeXMLNode(std::ostream& ss, const std::string& tag,
			std::initializer_list<std::tuple<std::string, std::variant<std::string, double, int>>> values);

protected:
	const PCTree* cur_tree;
	PCNode* cur_node;
};

struct LinearDrawer : virtual Drawer {
	PCTreeNodeArray<double>* widths;
	double width;
	double height;

	virtual void drawQNode(std::ostream& ss, double x, double y, double width, double height) = 0;

	void draw(const PCTree& tree, const Layout& positions, std::ostream& ss) override;

	virtual std::tuple<std::string, double> getTrianglePath(double cx, double cy, double size);

	virtual std::tuple<double, double, double, double> getQNodeSize(PCNode* node,
			const Layout& positions);
};

struct CircularDrawer : virtual Drawer {
	double radius;

	void draw(const PCTree& tree, const Layout& positions, std::ostream& ss) override;

	virtual void drawCNode(std::ostream& ss, double x, double y) = 0;
};

// /////////////////////////////////////////////////////////////////////////////////////////////////
// SVG /////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////

struct SVGDrawer : virtual Drawer {
	void writeTrailer(std::ostream& ss) override { ss << "</svg>"; }

	void drawLine(std::ostream& ss, double x1, double y1, double x2, double y2) override {
		writeXMLNode(ss, "line",
				{{"x1", x1}, {"y1", y1}, {"x2", x2}, {"y2", y2}, {"stroke", "black"}});
	}
};

struct SVGCircularDrawer : virtual SVGDrawer, virtual CircularDrawer {
	void writeHeader(std::ostream& ss) override {
		double act_rad = offset_x = offset_y = radius + (node_size / 2) + 1;
		double size = act_rad * 2;
		ss << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << size << "\" height=\"" << size
		   << "\">";
		writeXMLNode(ss, "circle",
				{{"cx", offset_x}, {"cy", offset_y}, {"r", radius}, {"fill", "transparent"},
						{"stroke", "#cccccc"}, {"pointer-events", "none"}});
	}

	void drawLeaf(std::ostream& ss, double cx, double cy, std::string label) override {
		writeXMLNode(ss, "circle",
				{{"cx", cx}, {"cy", cy}, {"r", node_size / 2}, {"fill", "white"},
						{"stroke", "#666666"}, {"data-leaves", label}});
		writeXMLNode(ss, "text",
				{{"x", cx}, {"y", cy}, {"text-anchor", "middle"}, {"dominant-baseline", "central"},
						{"text", label}});
	}

	void drawPNode(std::ostream& ss, double cx, double cy) override {
		writeXMLNode(ss, "circle",
				{{"cx", cx}, {"cy", cy}, {"r", node_size / 6}, {"fill", "black"},
						{"stroke", "black"}});
	}

	void drawCNode(std::ostream& ss, double cx, double cy) override {
		writeXMLNode(ss, "circle",
				{{"cx", cx}, {"cy", cy}, {"r", node_size * 0.65}, {"fill", "#ececec"},
						{"stroke", "black"}});
		writeXMLNode(ss, "circle",
				{{"cx", cx}, {"cy", cy}, {"r", node_size / 2}, {"fill", "transparent"},
						{"stroke", "black"}, {"pointer-events", "none"}});
		writeXMLNode(ss, "text",
				{{"x", (cx - 1)}, {"y", cy}, {"text-anchor", "middle"},
						{"dominant-baseline", "central"}, {"text", "C"}});
	}
};

struct SVGLinearDrawer : virtual SVGDrawer, virtual LinearDrawer {
	void writeHeader(std::ostream& ss) override {
		ss << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << width << "\" height=\""
		   << height << "\">";
	}

	void drawLeaf(std::ostream& ss, double cx, double cy, std::string label) override {
		auto [tri, mid_y] = getTrianglePath(cx, cy, node_size);
		writeXMLNode(ss, "polygon",
				{{"points", tri}, {"fill", "white"}, {"stroke", "black"}, {"data-leaves", label}});
		writeXMLNode(ss, "text",
				{{"x", cx}, {"y", mid_y}, {"text-anchor", "middle"},
						{"dominant-baseline", "central"}, {"text", label}});
	}

	void drawPNode(std::ostream& ss, double cx, double cy) override {
		writeXMLNode(ss, "circle",
				{{"cx", cx}, {"cy", cy}, {"r", node_size / 2}, {"fill", "#ececec"},
						{"stroke", "black"}, {"data-leaves", getLeaves(cur_node)}});
		writeXMLNode(ss, "text",
				{{"x", (cx + 0.4)}, {"y", cy}, {"text-anchor", "middle"},
						{"dominant-baseline", "central"}, {"text", "P"}});
	}

	void drawQNode(std::ostream& ss, double x, double y, double width, double height) override {
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
	virtual void writeStyle(std::ostream& ss) { }

	void writeHeader(std::ostream& ss) override {
		ss << "<?xml version=\"1.0\"?>\n<!DOCTYPE ipe SYSTEM \"ipe.dtd\">\n<ipe version=\"70218\" creator=\"pqtree.js\">\n";
		writeStyle(ss);
		ss << "<page>\n<layer name=\"alpha\"/>\n<view layers=\"alpha\" active=\"alpha\"/>"
		   << std::endl;
	}

	void writeTrailer(std::ostream& ss) override { ss << "</page>\n</ipe>" << std::endl; }

	void drawLine(std::ostream& ss, double x1, double y1, double x2, double y2) override {
		ss << "<path layer=\"alpha\">\n"
		   << x1 << " " << -y1 << " m\n"
		   << x2 << " " << -y2 << " l\n</path>" << std::endl;
	}

	virtual void drawMark(std::ostream& ss, double x, double y, std::string mark,
			std::initializer_list<std::tuple<std::string, std::string>> values = {}) {
		ss << "<use layer=\"alpha\" pos=\"" << x << " " << -y << "\" name=\"mark/" << mark
		   << "(sfx)\"";
		for (auto [key, value] : values) {
			ss << " " << key << "=\"" << value << "\"";
		}
		ss << "/>" << std::endl;
	}

	virtual void drawText(std::ostream& ss, double x, double y, std::string text) {
		writeXMLNode(ss, "text",
				{{"text", text}, {"pos", std::to_string(x) + " " + std::to_string(-y)},
						{"type", "label"}, {"halign", "center"}, {"valign", "center"},
						{"layer", "alpha"}})
				<< std::endl;
	}

	virtual std::string getCirclePath(double rad, double x, double y) {
		std::stringstream circ;
		circ << rad << " 0 0 " << rad << " " << x << " " << -y << " e";
		return circ.str();
	}
};

struct IPECircularDrawer : virtual IPEDrawer, virtual CircularDrawer {
	void writeHeader(std::ostream& ss) override {
		IPEDrawer::writeHeader(ss);
		double act_rad = offset_x = offset_y = radius + (node_size / 2) + 1;
		offset_y -= 2 * act_rad;
		writeXMLNode(ss, "path",
				{{"stroke", "black"}, {"text", getCirclePath(radius, offset_x, offset_y)},
						{"layer", "alpha"}})
				<< std::endl;
	}

	void writeStyle(std::ostream& ss) override;

	void drawLeaf(std::ostream& ss, double x, double y, std::string label) override {
		drawMark(ss, x, y, "leaf");
		drawText(ss, x, y, label);
	}

	void drawPNode(std::ostream& ss, double x, double y) override {
		drawMark(ss, x, y, "pnode", {{"fill", "black"}});
	}

	void drawCNode(std::ostream& ss, double x, double y) override { drawMark(ss, x, y, "cnode"); }
};

struct IPELinearDrawer : virtual IPEDrawer, virtual LinearDrawer {
	void writeStyle(std::ostream& ss) override;

	void drawLeaf(std::ostream& ss, double cx, double cy, std::string label) override {
		drawMark(ss, cx, cy, "leaf");
		drawText(ss, cx, cy + 16, label);
	}

	void drawPNode(std::ostream& ss, double cx, double cy) override {
		writeXMLNode(ss, "path",
				{{"stroke", "black"}, {"fill", "white"},
						{"text", getCirclePath(node_size / 2, cx, cy)}, {"layer", "alpha"}})
				<< std::endl;
		drawText(ss, cx, cy, "P");
	}

	void drawQNode(std::ostream& ss, double x, double y, double width, double height) override {
		ss << "<path stroke=\"black\" fill=\"white\" layer=\"alpha\">\n"
		   << x << " " << -y << " m\n"
		   << x << " " << (-y - height) << " l\n"
		   << (x + width) << " " << (-y - height) << " l\n"
		   << (x + width) << " " << -y << " l\nh\n</path>" << std::endl;
		drawText(ss, x + width / 2, y + height / 2, "Q");
	}
};

// /////////////////////////////////////////////////////////////////////////////////////////////////
// Tikz ////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////

struct TikzDrawer : virtual Drawer {
	double scale = 0.02;

	double s(double v) { return v * scale; }

	void writeTrailer(std::ostream& ss) override { ss << "\\end{tikzpicture}"; }

	void drawLine(std::ostream& ss, double x1, double y1, double x2, double y2) override {
		ss << "\\draw (" << s(x1) << "," << s(-y1) << ") -- (" << s(x2) << "," << s(-y2) << ");"
		   << std::endl;
	}

	virtual void drawText(std::ostream& ss, double x, double y, std::string text) {
		ss << "\\node at (" << x << "," << y << ") {" << text << "};" << std::endl;
	}
};

struct TikzCircularDrawer : virtual TikzDrawer, virtual CircularDrawer {
	void writeHeader(std::ostream& ss) override {
		ss << "\\begin{tikzpicture}[leaf/.style={fill=white},pnode/.style={fill=black},"
			  "cnode/.style={fill=black!10, double distance=2.5pt},"
			  "background/.style={color=black!20}]\n";
		double act_rad = offset_x = offset_y = radius + (node_size / 2);
		ss << "\\draw[background] (" << s(offset_x) << "," << s(-offset_y)
		   << ") circle [radius=" << s(radius) << "];" << std::endl;
	}

	void drawLeaf(std::ostream& ss, double cx, double cy, std::string label) override {
		ss << "\\draw[leaf] (" << s(cx) << "," << s(-cy) << ") circle [radius=" << s(node_size / 2)
		   << "];" << std::endl;
		drawText(ss, s(cx), s(-cy), label);
	}

	void drawPNode(std::ostream& ss, double cx, double cy) override {
		ss << "\\draw[pnode] (" << s(cx) << "," << s(-cy) << ") circle [radius=" << s(node_size / 6)
		   << "];" << std::endl;
	}

	void drawCNode(std::ostream& ss, double cx, double cy) override {
		ss << "\\draw[cnode] (" << s(cx) << "," << s(-cy)
		   << ") circle [radius=" << s(node_size * 0.65) << "];" << std::endl;
		drawText(ss, s(cx), s(-cy), "C");
	}
};

struct TikzLinearDrawer : virtual TikzDrawer, virtual LinearDrawer {
	void writeHeader(std::ostream& ss) override {
		ss << "\\begin{tikzpicture}[leaf/.style={fill=white},pnode/"
			  ".style={fill=black!10},qnode/.style={pnode}]"
		   << std::endl;
	}

	void drawLeaf(std::ostream& ss, double cx, double cy, std::string label) override {
		auto [tri, mid_y] = getTrianglePath(s(cx), s(cy), s(node_size));
		tri = std::regex_replace(tri, std::regex(" "), ") -- (");
		tri = std::regex_replace(tri, std::regex(","), ",-");
		ss << "\\draw[leaf](" << tri << ") -- cycle;" << std::endl;
		drawText(ss, s(cx), -mid_y, label);
	}

	void drawPNode(std::ostream& ss, double cx, double cy) override {
		ss << "\\draw[pnode] (" << s(cx) << "," << s(-cy) << ") circle [radius=" << s(node_size / 2)
		   << "];" << std::endl;
		drawText(ss, s(cx), s(-cy), "P");
	}

	void drawQNode(std::ostream& ss, double x, double y, double width, double height) override {
		ss << "\\draw[qnode] (" << s(x) << "," << s(-y) << ") rectangle (" << s(x + width) << ","
		   << s(-y - height) << ");" << std::endl;
		drawText(ss, s(x + width / 2), s(-y - height / 2), "Q");
	}
};
