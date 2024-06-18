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
} Filter;

typedef enum {
  LOW = 0,
  MEDIUM = 1,
  HIGH = 2,
} Priority;

typedef struct {
  bool completed;
  char * description, * date;
  Priority priority;
} task;

static int win_w = 920, win_h = 420;
static Filter current_filter = 0;
static LfFont title_font;
static LfFont regular_font;
static LfFont bold_font;

static task * tasks[1024];
static uint32_t num_tasks = 0;

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
  lf_set_no_render(true);
  {
    float filters_w = 0.0f;
    float ptr_x_before = lf_get_ptr_x();

    for (int i = 0; i < num_filters; ++i)
      lf_button(filters[i]);

    filters_w = lf_get_ptr_x() - ptr_x_before
      - btn_props.margin_left - btn_props.margin_right
      - (btn_props.padding * 2 * (num_filters - 1));

    lf_set_ptr_x_absolute(win_w - filters_w - (WIN_PADDING * 2));
  }
  lf_set_no_render(false);

  /* actually print the buttons */
  for (int i = 0; i < num_filters; ++i) {
    btn_props.color = LF_NO_COLOR;
    btn_props.text_color = (LfColor) { 255, 255, 255, 255 };

    if (current_filter == (Filter) i) {
      btn_props.color = (LfColor) { 180, 180, 200, 255 };
      btn_props.text_color = (LfColor) { 11, 11, 11, 255 };
    }

    lf_push_style_props(btn_props);
    if (lf_button(filters[i]) == LF_CLICKED)
      current_filter = (Filter) i;
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

  task * new_task = (task *) malloc(sizeof(* new_task));
  new_task->completed = false;
  new_task->priority = LOW;
  new_task->date = "empty";
  new_task->description = "Code something";
  tasks[num_tasks++] = new_task;

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
    {
      const float priority_size = 13.0f;
      LfColor priority_color;
      LfUIElementProps div_props = lf_get_theme().div_props;

      lf_next_line();

      div_props.margin_left = 0.0f;
      div_props.margin_top = 0.0f;
      div_props.margin_right = 0.0f;
      div_props.margin_bottom = 0.0f;
      div_props.padding = (float) WIN_PADDING;
      lf_push_style_props(div_props);

      lf_div_begin(
        ((vec2s) { lf_get_ptr_x() + WIN_PADDING, lf_get_ptr_y() + (WIN_PADDING * 2) }),
        ((vec2s) { win_w - (WIN_PADDING * 4.0f), win_h - lf_get_ptr_y() - (WIN_PADDING * 4.0f) }),
        true
      );

      lf_pop_style_props();

      /* draw priority badges */
      for (int i = 0; i < num_tasks; ++i, lf_next_line()) {
        task * t = tasks[i];

        switch (t->priority) {
          case LOW:
            priority_color = (LfColor) { 14, 168, 239, 255 };
            break;
          case MEDIUM:
            priority_color = (LfColor) { 239, 157, 14, 255 };
            break;
          case HIGH:
            priority_color = (LfColor) { 239, 14, 48, 255 };
        }

        lf_rect(priority_size, priority_size, priority_color, 4.0f);
      }

      lf_div_end();
    }

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
