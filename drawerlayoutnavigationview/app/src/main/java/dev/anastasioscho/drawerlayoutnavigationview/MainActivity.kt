package dev.anastasioscho.drawerlayoutnavigationview

import android.content.res.Configuration
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.MenuItem
import android.widget.Toast
import androidx.appcompat.app.ActionBarDrawerToggle
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity()
{
    private lateinit var toggle: ActionBarDrawerToggle

    override fun onCreate(savedInstanceState: Bundle?)
    {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.activity_main)

        setupAppBar()
        setupControls()
    }

    override fun onPostCreate(savedInstanceState: Bundle?)
    {
        super.onPostCreate(savedInstanceState)

        toggle.syncState()
    }

    override fun onConfigurationChanged(newConfig: Configuration?)
    {
        super.onConfigurationChanged(newConfig)

        toggle.onConfigurationChanged(newConfig)
    }

    override fun onOptionsItemSelected(item: MenuItem?): Boolean
    {
        if (toggle.onOptionsItemSelected(item)) {
            return true
        }

        return super.onOptionsItemSelected(item)
    }

    private fun setupAppBar()
    {
        setSupportActionBar(toolbar)

        supportActionBar?.setDisplayHomeAsUpEnabled(true)

        toggle = ActionBarDrawerToggle(this, drawerLayout, R.string.open_drawer, R.string.close_drawer)
        drawerLayout.addDrawerListener(toggle)
    }

    private fun setupControls()
    {
        navigationView.setNavigationItemSelectedListener {menuItem: MenuItem -> Boolean
            when (menuItem.itemId) {
                R.id.menu_item_1 -> Toast.makeText(this, "Item 1", Toast.LENGTH_SHORT).show()
                R.id.menu_item_2 -> Toast.makeText(this, "Item 2", Toast.LENGTH_SHORT).show()
                R.id.menu_item_3 -> Toast.makeText(this, "Item 3", Toast.LENGTH_SHORT).show()
                R.id.menu_item_4 -> Toast.makeText(this, "Item 4", Toast.LENGTH_SHORT).show()
                R.id.menu_item_5 -> Toast.makeText(this, "Item 5", Toast.LENGTH_SHORT).show()
                R.id.menu_item_6 -> Toast.makeText(this, "Item 6", Toast.LENGTH_SHORT).show()
                R.id.menu_item_7 -> Toast.makeText(this, "Item 7", Toast.LENGTH_SHORT).show()
            }

            drawerLayout.closeDrawers()

            return@setNavigationItemSelectedListener true
        }
    }
}
