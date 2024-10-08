#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <cglm/types-struct.h>
#include <leif/leif.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WM_CLASS "TodoC" /* window title */
#define WIN_PADDING 20
#define TASK_TEXT_BUFFER_SIZE 512

typedef enum {
  DASHBOARD = 0,
  NEW_TASK
} GuiTab;

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

static uint16_t win_w = 640, win_h = 380;
static Filter current_filter = ALL;
static GuiTab current_tab = DASHBOARD;
static LfFont title_font;
static LfFont new_task_font_bold;
static LfFont new_task_font_regular;
static LfFont filter_font;
static LfFont task_font;
static LfTexture trash_texture;
static LfTexture back_texture;
static LfInputField new_task_input;
static char new_task_input_value[TASK_TEXT_BUFFER_SIZE];

static task * tasks[1024];
static uint16_t num_tasks = 0;

static char *
get_cmd_output(const char * cmd) {
  FILE * fp;
  char buffer[1024];
  char * std_out = NULL;
  size_t std_out_size = 0;

  /* open new pipe for cmd */
  fp = popen(cmd, "r");
  if (fp == NULL) {
    printf("Failed to run command %s \n", cmd);
    return NULL;
  }

  /* read cmd output */
  while (fgets(buffer, sizeof(buffer), fp) != (void *) '\0') {
    size_t buffer_len = strlen(buffer);
    char * temp = realloc(std_out, std_out_size + buffer_len + 1);

    if (temp == NULL) {
      printf("Memory allocation failed\n");
      free(std_out);
      pclose(fp);
      return NULL;
    }

    std_out = temp;
    strcpy(std_out + std_out_size, buffer);
    std_out_size += buffer_len;
  }

  pclose(fp);

  return std_out;
}

static LfUIElementProps
get_btn_props(bool is_main_tab)
{
  LfUIElementProps btn_props;

  if (is_main_tab)
    btn_props = lf_get_theme().button_props;
  else
    btn_props = lf_get_theme().image_props;

  btn_props.padding = 7.5f;
  btn_props.margin_left = is_main_tab ? 0.f : 10.f;
  btn_props.margin_right = 0.0f;
  btn_props.margin_top = is_main_tab ? 0.f : 10.f;
  btn_props.margin_bottom = 0.0f;
  btn_props.color = (LfColor) { 150, 150, 215, 255 };
  btn_props.hover_color = (LfColor) { 111, 111, 215, 255 };
  btn_props.border_width = 0.f;
  btn_props.corner_radius = 6.7f;

  if (is_main_tab)
    btn_props.text_color = LF_WHITE;
  else
    btn_props.text_color = LF_BLACK;

  return btn_props;
}

static void
render_title(char *title)
{
  lf_push_font(&title_font);
  lf_text(title);
  lf_pop_font();
}

static void
render_header(char *title)
{
  const float btn_w = 85.f;

  LfUIElementProps btn_props = get_btn_props(current_tab == DASHBOARD);
  lf_push_style_props(btn_props);

  switch (current_tab) {
    case DASHBOARD:
      render_title(title);

      lf_set_ptr_x_absolute(win_w - (WIN_PADDING * 2.0f) - btn_w);

      btn_props.text_color = (LfColor) { 0, 0, 0, 255 };
      lf_push_style_props(btn_props);
      lf_push_font(&new_task_font_bold);

      if (lf_button_fixed("New task", btn_w, -1) == LF_CLICKED)
        current_tab = NEW_TASK;
      lf_pop_style_props();
      lf_pop_font();

      break;

    case NEW_TASK:
      if (/* render back_button */ lf_image_button(((LfTexture) {
        .id = back_texture.id,
        .width = 20.f,
        .height = 20.f
      })) == LF_CLICKED)
        current_tab = DASHBOARD;

      btn_props.text_color = (LfColor) { 255, 255, 255, 255 };
      btn_props.margin_top = 6.9f;
      lf_push_style_props(btn_props);

      render_title(title);
      lf_pop_style_props();
  }

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

static void
render_tasks(void)
{
  LfUIElementProps div_props = lf_get_theme().div_props;
  div_props.margin_left = 0.0f;
  div_props.margin_top = 0.0f;
  div_props.margin_right = 0.0f;
  div_props.margin_bottom = 0.0f;
  lf_push_style_props(div_props);

  lf_div_begin(
    ((vec2s) { lf_get_ptr_x(), lf_get_ptr_y() + 10.f }),
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

    { /* click on priority to change it */
      bool clicked_priority = lf_hovered(
        (vec2s) { lf_get_ptr_x(), lf_get_ptr_y() + inc_y },
        (vec2s) { priority_size, priority_size }
      ) && lf_mouse_button_went_down(GLFW_MOUSE_BUTTON_LEFT);

      if (clicked_priority) {
        if (t->priority + 1 >= HIGH + 1)
          t->priority = 0;
        else
          ++t->priority;
      }
    }

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
      lf_set_ptr_y_absolute(ptr_y - inc_y + 1.f);
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

static int
compare_task_priority(const void * a, const void * b)
{
  task * task_a = * (task **) a;
  task * task_b = * (task **) b;

  return task_b->priority - task_a->priority; /* the higher priority task will come before a lower priority task */
}

static void
sort_tasks_by_priority(void)
{
  qsort(tasks, num_tasks, sizeof(task *), compare_task_priority);
}

static void
render_new_task(void)
{
  lf_next_line();

  LfUIElementProps field_description_props = lf_get_theme().text_props;
  field_description_props.margin_top = 20.f;

  { /* render new_task input text */
    lf_push_font(&new_task_font_bold);
    lf_push_style_props(field_description_props);
    lf_text("Task");

    lf_pop_style_props();
    lf_pop_font();

    lf_next_line();
    LfUIElementProps input_props = lf_get_theme().inputfield_props;
    input_props.padding = 11.f;
    input_props.color = lf_color_from_zto((vec4s) { 0.05f, 0.05f, 0.05f, 1.0f });
    input_props.corner_radius = 2.f;
    input_props.text_color = (LfColor) { 255, 255, 255, 255 };
    input_props.border_width = 1.f;
    input_props.border_color = new_task_input.selected ? LF_WHITE : (LfColor) { 150, 150, 150, 255 };
    input_props.margin_top = 7.f;
    input_props.margin_bottom = 7.f;

    lf_push_font(&new_task_font_regular);
    lf_push_style_props(input_props);
    lf_input_text(&new_task_input);

    lf_pop_style_props();
    lf_pop_font();
  }

  static int32_t selected_priority = -1;
  { /* render priority dropdown */
    static bool opened = false;
    static const char * items[3] = {
      "Low",
      "Medium",
      "High"
    };

    LfUIElementProps dropdown_props = lf_get_theme().button_props;
    dropdown_props.color = LF_NO_COLOR;
    dropdown_props.text_color = LF_WHITE;
    dropdown_props.border_width = 0.f;
    dropdown_props.corner_radius = 0.f;
    dropdown_props.corner_radius = 2.f;
    dropdown_props.margin_top = 0.f;
    dropdown_props.margin_left = 0.f;
    dropdown_props.margin_right = 0.f;
    dropdown_props.margin_bottom = 0.f;
    dropdown_props.padding = 5.5f;

    lf_push_font(&new_task_font_bold);
    lf_push_style_props(dropdown_props);
    lf_dropdown_menu(items, "Priority", 3, 140, 80, &selected_priority, &opened);
    lf_pop_font();
    lf_pop_style_props();
  }

  { /* render ADD task button */
    bool valid_task = strlen(new_task_input_value) && selected_priority != -1;
    const char * text = "Add";
    const float width = 80.f;

    LfUIElementProps btn_props = get_btn_props(false);

    if (!valid_task) {
      LfColor disabled_color = (LfColor) { 100, 100, 100, 255 };
      btn_props.color = disabled_color;
      btn_props.hover_color = disabled_color;
    }

    lf_set_ptr_x_absolute(win_w - (7.f * WIN_PADDING));
    lf_set_ptr_y_absolute(WIN_PADDING * 1.1f);
    lf_push_style_props(btn_props);
    lf_push_font(&new_task_font_bold);

    if (
      lf_button_fixed(text, width, -1) == LF_CLICKED && valid_task
      || lf_key_went_down(GLFW_KEY_ENTER) && valid_task
    ) {
      task * new_task = (task *) malloc(sizeof(* new_task));
      new_task->completed = false;
      new_task->priority = (Priority) selected_priority;
      new_task->date = get_cmd_output("bash ./script/date.sh && bash ./script/hour.sh");

      /* copy input buffer value into new variable */
      char * new_description = malloc(strlen(new_task_input_value));
      strcpy(new_description, new_task_input_value);

      new_task->description = new_description;
      tasks[num_tasks++] = new_task;

      /* clear input buffer string */
      memset(new_task_input_value, 0, TASK_TEXT_BUFFER_SIZE);
      /* reset priority */
      selected_priority = -1;

      sort_tasks_by_priority();

      current_tab = DASHBOARD;
    }

    lf_pop_style_props();
    lf_pop_font();
  }
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
  new_task_font_bold = lf_load_font("./font/RecMonoCasualNerdFont-Bold.ttf", 20);
  new_task_font_regular = lf_load_font("./font/RecMonoCasualNerdFont-Regular.ttf", 20);
  filter_font = lf_load_font("./font/RecMonoCasualNerdFont-Bold.ttf", 18);
  task_font = lf_load_font("./font/FreeSansBold.otf", 16);

  trash_texture = lf_load_texture("./icon/trash.png", true, LF_TEX_FILTER_LINEAR);
  back_texture = lf_load_texture("./icon/back.png", true, LF_TEX_FILTER_LINEAR);

  memset(new_task_input_value, 0, TASK_TEXT_BUFFER_SIZE);
  new_task_input = (LfInputField) {
    .width = (win_w - WIN_PADDING * 4.f),
    .buf = new_task_input_value,
    .buf_size = TASK_TEXT_BUFFER_SIZE,
    .placeholder = (char *) "Is there something to do?"
  };

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

    lf_begin();

    lf_div_begin(
      ((vec2s) { WIN_PADDING, WIN_PADDING }),
      ((vec2s) { win_w - WIN_PADDING * 2.0f, win_h - WIN_PADDING * 2.0f }),
      true
    );

    switch (current_tab) {
      case DASHBOARD:
        render_header(WM_CLASS);
        render_filters();
        render_tasks();
        break;
      case NEW_TASK:
        render_header("Create new task");
        render_new_task();
        break;
    }

    lf_div_end();
    lf_end();

    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  lf_free_font(&title_font);
  lf_free_font(&new_task_font_bold);
  lf_free_font(&new_task_font_regular);
  lf_free_font(&filter_font);
  lf_free_font(&task_font);

  lf_free_texture(&trash_texture);
  lf_free_texture(&back_texture);

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
