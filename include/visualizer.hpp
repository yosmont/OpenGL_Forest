#ifndef VISUALIZER_HPP
#define VISUALIZER_HPP

enum { inVisualizerNamespace = false };
namespace visualizer { enum { inVisualizerNamespace = true }; }

#define BEGIN_VISUALIZER_NAMESPACE static_assert(!inVisualizerNamespace, "visualizer namespace not previously closed"); namespace visualizer {
#define END_VISUALIZER_NAMESPACE static_assert(inVisualizerNamespace, "visualizer namespace not previously opened"); }

#endif // !VISUALIZER_HPP
