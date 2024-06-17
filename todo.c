#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <leif/leif.h>

#define WM_CLASS "TodoC" /* window title */
#define WIN_PADDING 20

typedef enum {
  ALL = 0,
  TODO = 1,
  IN_PROGRESS = 2,
  COMPLETED = 3,
} EntryFilter;

static int win_w = 920, win_h = 420;
static EntryFilter current_filter = 0;
static LfFont title_font;
static LfFont regular_font;
static LfFont bold_font;

static void
render_header(void)
{
  const float btn_w = 100.0f;

  lf_push_font(&title_font);
  lf_text(WM_CLASS);
  lf_pop_font();

  LfUIElementProps btn_props = lf_get_theme().button_props;
  btn_props.margin_left = 0.0f;
  btn_props.margin_right = 0.0f;
  btn_props.margin_top = 0.0f;
  btn_props.margin_bottom = 0.0f;
  btn_props.color = (LfColor) { 211, 111, 135, 255 };
  btn_props.border_width = 0.0f;
  btn_props.corner_radius = 2.0f;

  lf_push_style_props(btn_props);
  lf_set_ptr_x_absolute(win_w - (WIN_PADDING * 2.0f) - btn_w);

  lf_push_font(&regular_font);
  lf_button_fixed("New todo", btn_w, -1);
  lf_pop_font();

  lf_pop_style_props();
}

static void
render_filters(void)
{
  const int num_filters = 4;
  static const char * filters[] = { "ALL", "TODO", "IN PROGRESS", "COMPLETED" };

  LfUIElementProps btn_props = lf_get_theme().button_props;
  btn_props.margin_top = WIN_PADDING;
  btn_props.padding = 6.0f;
  btn_props.border_width = 0.0f;
  btn_props.corner_radius = 6.0f;

  lf_push_font(&bold_font);
  lf_next_line();

  /* float to right */
  {
    float filters_w = 0.0f;
    float ptr_x_before = lf_get_ptr_x();

    lf_set_no_render(true);
    for (int i = 0; i < num_filters; ++i)
      lf_button(filters[i]);
    lf_set_no_render(false);

    filters_w = lf_get_ptr_x() - ptr_x_before
      - btn_props.margin_left - btn_props.margin_right
      - (btn_props.padding * 2 * (num_filters - 1));

    lf_set_ptr_x_absolute(win_w - filters_w - (WIN_PADDING * 2));
  }

  /* actually print the buttons */
  for (int i = 0; i < num_filters; ++i) {
    btn_props.color = LF_NO_COLOR;
    btn_props.text_color = (LfColor) { 255, 255, 255, 255 };

    if (current_filter == (EntryFilter) i) {
      btn_props.color = (LfColor) { 180, 180, 200, 255 };
      btn_props.text_color = (LfColor) { 11, 11, 11, 255 };
    }

    lf_push_style_props(btn_props);
    if (lf_button(filters[i]) == LF_CLICKED)
      current_filter = (EntryFilter) i;
    lf_pop_style_props();
  }

  lf_pop_font();
}

int
main(void)
{
  glfwInit();

  GLFWwindow * window = glfwCreateWindow(win_w, win_h, WM_CLASS, NULL, NULL);

  glfwMakeContextCurrent(window);

  lf_init_glfw(win_w, win_h, window);

  LfTheme theme = lf_get_theme();
  theme.div_props.color = LF_NO_COLOR;
  lf_set_theme(theme);

  title_font = lf_load_font("./font/SpaceMonoNerdFont-Bold.ttf", 35);
  regular_font = lf_load_font("./font/SpaceMonoNerdFont-Regular.ttf", 25);
  bold_font = lf_load_font("./font/SpaceMonoNerdFont-Bold.ttf", 25);

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

    lf_begin();

    lf_div_begin(
      ((vec2s) { WIN_PADDING, WIN_PADDING }),
      ((vec2s) { win_w - WIN_PADDING * 2.0f, win_h - WIN_PADDING * 2.0f }),
      true
    );

    render_header();
    render_filters();

    lf_div_end();
    lf_end();

    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  lf_free_font(&title_font);
  lf_free_font(&regular_font);
  lf_free_font(&bold_font);

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
