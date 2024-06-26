#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <leif/leif.h>
#include <stdint.h>
#include <stdio.h>

#define WM_CLASS "TodoC" /* window title */
#define WIN_PADDING 20

typedef enum {
  ALL = 0,
  IN_PROGRESS,
  COMPLETED
} Filter;

typedef enum {
  LOW = 0,
  MEDIUM,
  HIGH
} Priority;

typedef struct {
  bool completed;
  char * description, * date;
  Priority priority;
} task;

static uint16_t win_w = 640, win_h = 320;
static Filter current_filter = 0;
static LfFont title_font;
static LfFont newtask_font;
static LfFont filter_font;
static LfFont task_font;
static LfTexture trash_texture;

static task * tasks[1024];
static uint16_t num_tasks = 0;

static void
render_header(void)
{
  const float btn_w = 85.f;

  lf_push_font(&title_font);
  lf_text(WM_CLASS);
  lf_pop_font();

  LfUIElementProps btn_props = lf_get_theme().button_props;
  btn_props.padding = 7.5f;
  btn_props.margin_left = 0.0f;
  btn_props.margin_right = 0.0f;
  btn_props.margin_top = 0.0f;
  btn_props.margin_bottom = 0.0f;
  btn_props.color = (LfColor) { 150, 150, 215, 255 };
  btn_props.hover_color = (LfColor) { 111, 111, 215, 255 };
  btn_props.border_width = 0.f;
  btn_props.corner_radius = 6.7f;

  lf_push_style_props(btn_props);
  lf_set_ptr_x_absolute(win_w - (WIN_PADDING * 2.0f) - btn_w);

  lf_push_font(&newtask_font);
  lf_button_fixed("New task", btn_w, -1);
  lf_pop_font();

  lf_pop_style_props();
}

static void
render_filters(void)
{
  const uint8_t num_filters = 3;
  static const char * filters[] = { "all", "in progress", "completed" };

  LfUIElementProps btn_props = lf_get_theme().button_props;
  btn_props.margin_top = WIN_PADDING;
  btn_props.padding = 6.0f;
  btn_props.border_width = 0.0f;
  btn_props.corner_radius = 6.0f;

  lf_push_font(&filter_font);
  lf_next_line();

  /* actually print the buttons */
  for (uint8_t i = 0; i < num_filters; ++i) {
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
  lf_next_line();
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

  title_font = lf_load_font("./font/RecMonoCasualNerdFont-Bold.ttf", 35);
  newtask_font = lf_load_font("./font/RecMonoCasualNerdFont-Regular.ttf", 18);
  filter_font = lf_load_font("./font/RecMonoCasualNerdFont-Bold.ttf", 16);
  task_font = lf_load_font("./font/FreeSansBold.otf", 16);

  trash_texture = lf_load_texture("./icon/trash.png", true, LF_TEX_FILTER_LINEAR);

  task * new_task = (task *) malloc(sizeof(* new_task));
  new_task->completed = false;
  new_task->priority = LOW;
  new_task->date = "empty";
  new_task->description = "Code something";
  tasks[num_tasks++] = new_task;
  task * task2 = (task *) malloc(sizeof(* task2));
  task2->completed = true;
  task2->priority = MEDIUM;
  task2->date = "new date";
  task2->description = "Ur momma";
  tasks[num_tasks++] = task2;

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
      LfUIElementProps div_props = lf_get_theme().div_props;
      div_props.margin_left = 0.0f;
      div_props.margin_top = 0.0f;
      div_props.margin_right = 0.0f;
      div_props.margin_bottom = 0.0f;
      lf_push_style_props(div_props);

      lf_div_begin(
        ((vec2s) { lf_get_ptr_x(), lf_get_ptr_y() }),
        ((vec2s) { win_w - (WIN_PADDING * 2.0f), win_h - lf_get_ptr_y() - WIN_PADDING }),
        true
      );

      lf_pop_style_props();
      lf_push_font(&task_font);

      uint16_t num_rendered = 0;

      /* draw task */
      for (uint16_t i = 0; i < num_tasks; ++i, lf_next_line()) {
        task * t = tasks[i];

        if (current_filter == IN_PROGRESS && t->completed)
          continue;
        if (current_filter == COMPLETED && !t->completed)
          continue;

        ++num_rendered;

        float ptr_y = lf_get_ptr_y() + 5.f;
        const float inc_y = 16.5f;
        const float priority_size = 11.f;
        float margin_left = 15.f;

        { /* draw priority badge */
          LfColor priority_color;

          switch (t->priority) {
            case LOW:
              priority_color = (LfColor) { 14, 168, 239, 255 };
              break;
            case MEDIUM:
              priority_color = (LfColor) { 239, 200, 14, 255 };
              break;
            case HIGH:
              priority_color = (LfColor) { 239, 14, 48, 255 };
          }

          lf_set_ptr_y_absolute(ptr_y += inc_y);
          lf_rect(priority_size, priority_size, priority_color, 6.f);
        }

        { /* draw remove task trash can button */
          LfUIElementProps trash_props = lf_get_theme().button_props;
          trash_props.color = LF_NO_COLOR;
          trash_props.border_width = 0.f;
          trash_props.padding = 0.f;
          trash_props.margin_top = 0.f;
          trash_props.margin_left = 0.f;
          trash_props.margin_right = 0.f;
          trash_props.margin_bottom = 0.f;
          lf_push_style_props(trash_props);

          lf_set_ptr_y_absolute(ptr_y - 5.f);
          lf_set_ptr_x(margin_left *= 1.2f);

          if (lf_image_button(
            ((LfTexture) { .id = trash_texture.id, .width = 20.f, .height = 20.f })
          ) == LF_CLICKED) {
            for (uint8_t task_index = i; task_index < num_tasks - 1; ++task_index)
              *( tasks + i ) = *( tasks + i + 1 );
            --num_tasks;
          }

          lf_pop_style_props();
        }

        { /* draw completed checkbox */
          LfUIElementProps cb_props = lf_get_theme().checkbox_props;
          cb_props.color = LF_NO_COLOR;
          cb_props.border_width = .5f;
          cb_props.border_color = (LfColor) { 66, 66, 66, 255 };
          cb_props.corner_radius = 2.f;
          lf_push_style_props(cb_props);

          lf_set_ptr_x(margin_left *= 2.3f);
          lf_set_ptr_y_absolute(ptr_y - inc_y);
          lf_checkbox("", &tasks[i]->completed, LF_NO_COLOR, ((LfColor) { 65, 167, 204, 255 }));
          lf_pop_style_props();

          lf_next_line();
        }

        { /* draw description */
          lf_set_ptr_y_absolute(ptr_y - inc_y);
          lf_set_ptr_x(margin_left *= 2.f);
          lf_text(tasks[i]->description);
        }

        { /* draw date */
          LfUIElementProps date_props = lf_get_theme().text_props;
          date_props.text_color = (LfColor) { 140, 140, 140, 255 };
          lf_push_style_props(date_props);

          lf_set_ptr_y_absolute(ptr_y);
          lf_set_ptr_x(margin_left);
          lf_text(tasks[i]->date);
          lf_pop_style_props();
        }

        ptr_y += inc_y * 2;
      }

      if (!num_rendered) {
        LfUIElementProps notask_props = lf_get_theme().text_props;
        notask_props.text_color = (LfColor) { 177, 177, 177, 255 };
        lf_push_style_props(notask_props);

        lf_push_font(&filter_font);
        lf_text("There is no tasks here... :(");
        lf_pop_font(&filter_font);

        lf_pop_style_props();
      }

      lf_pop_font();
      lf_div_end();
    }

    lf_div_end();
    lf_end();

    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  lf_free_font(&title_font);
  lf_free_font(&newtask_font);
  lf_free_font(&filter_font);
  lf_free_font(&task_font);

  lf_free_texture(&trash_texture);

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
